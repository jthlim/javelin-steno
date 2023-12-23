//---------------------------------------------------------------------------

#include "pattern_component.h"
#include "pattern_quick_reject.h"
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

void *PatternComponent::operator new(size_t size) {
  return PoolAllocate::operator new(size);
}

//---------------------------------------------------------------------------

bool PatternComponent::CallNext(const char *p, PatternContext &context) const {
  return !next || next->Match(p, context);
}

void PatternComponent::RemoveEpsilon() {
  while (next && next->IsEpsilon()) {
    next = next->next;
  }
  if (next) {
    next->RemoveEpsilon();
  }
}

void PatternComponent::UpdateQuickReject(
    PatternQuickReject &quickReject) const {
  if (!next) {
    return;
  }
  next->UpdateQuickReject(quickReject);
}

bool EpsilonPatternComponent::Match(const char *p,
                                    PatternContext &context) const {
  return CallNext(p, context);
}

bool AnyPatternComponent::Match(const char *p, PatternContext &context) const {
  if (*p == 0) {
    return false;
  }
  return CallNext(p + 1, context);
}

bool AnyStarPatternComponent::Match(const char *p,
                                    PatternContext &context) const {
  const PatternComponent *localNext = GetNext();
  if (!localNext) {
    return true;
  }

  const char *start = p;
  while (*p) {
    ++p;
  }

  while (p >= start) {
    if (localNext->Match(p, context)) {
      return true;
    }
    --p;
  }
  return false;
}

bool BackReferencePatternComponent::Match(const char *p,
                                          PatternContext &context) const {
  const char *compareP = context.captureList[index * 2];
  const char *comparedEnd = context.captureList[index * 2 + 1];

  while (compareP < comparedEnd) {
    if (*p != *compareP) {
      return false;
    }
    ++p;
    ++compareP;
  }

  return CallNext(p, context);
}

bool CharacterSetComponent::Match(const char *p,
                                  PatternContext &context) const {
  uint8_t c = *(uint8_t *)p;
  if (c >= 128) {
    return false;
  }
  unsigned int index = c / 8;
  unsigned int bit = 1 << (c & 7);
  if ((mask[index] & bit) == 0) {
    return false;
  }

  return CallNext(p + 1, context);
}

bool CapturePatternComponent::Match(const char *p,
                                    PatternContext &context) const {

  const char *previous = context.captureList[index];
  context.captureList[index] = p;
  bool result = CallNext(p, context);
  if (!result) {
    context.captureList[index] = previous;
  }
  return result;
}

bool BranchPatternComponent::Match(const char *p,
                                   PatternContext &context) const {
  return branch->Match(p, context) || CallNext(p, context);
}

void BranchPatternComponent::RemoveEpsilon() {
  if (processed) {
    return;
  }
  processed = true;

  while (branch && branch->IsEpsilon()) {
    branch = branch->next;
  }
  if (branch) {
    branch->RemoveEpsilon();
  }

  PatternComponent::RemoveEpsilon();
}

bool StartOfLinePatternComponent::Match(const char *p,
                                        PatternContext &context) const {
  return (p == context.start) && CallNext(p, context);
}

bool EndOfLinePatternComponent::Match(const char *p,
                                      PatternContext &context) const {
  return (*p == '\0') && CallNext(p, context);
}

//---------------------------------------------------------------------------

void BytePatternComponent::UpdateQuickReject(
    PatternQuickReject &quickReject) const {
  quickReject.Update(byte);
  PatternComponent::UpdateQuickReject(quickReject);
}

bool BytePatternComponent::Match(const char *p, PatternContext &context) const {
  if (*p != byte) {
    return false;
  }
  return CallNext(p + 1, context);
}

//---------------------------------------------------------------------------

void LiteralPatternComponent::UpdateQuickReject(
    PatternQuickReject &quickReject) const {
  quickReject.Update(text);
  PatternComponent::UpdateQuickReject(quickReject);
}

bool LiteralPatternComponent::Match(const char *p,
                                    PatternContext &context) const {
  const char *pText = text;
  for (;;) {
    if (*p != *pText) {
      return false;
    }
    ++p;
    ++pText;

    if (*pText == '\0') {
      return CallNext(p, context);
    }
  }
}

//---------------------------------------------------------------------------

ContainerPatternComponent::ContainerPatternComponent(
    PatternComponent *initialComponent) {
  componentCount = 1;
  components = (PatternComponent **)malloc(4 * sizeof(PatternComponent *));
  components[0] = initialComponent;
}

void ContainerPatternComponent::Add(PatternComponent *component) {
  size_t capacity = (componentCount + 3) & -4;
  if (componentCount == capacity) {
    PatternComponent **newComponents = (PatternComponent **)malloc(
        (capacity + 4) * sizeof(PatternComponent *));

    memcpy(newComponents, components,
           capacity * sizeof(PatternComponent *)); // NOLINT

    free(components);
    components = newComponents;
  }
  components[componentCount++] = component;
}

void ContainerPatternComponent::RemoveEpsilon() {
  for (size_t i = 0; i < componentCount; ++i) {
    components[i]->RemoveEpsilon();
  }
}

//---------------------------------------------------------------------------

bool AlternatePatternComponent::Match(const char *p,
                                      PatternContext &context) const {
  for (size_t i = 0; i < componentCount; ++i) {
    if (components[i]->Match(p, context)) {
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------
