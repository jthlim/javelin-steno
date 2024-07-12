//---------------------------------------------------------------------------

#include "pattern_component.h"
#include "pattern_quick_reject.h"
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

SuccessPatternComponent SuccessPatternComponent::instance;

//---------------------------------------------------------------------------

void *PatternComponent::operator new(size_t size) {
  return PoolAllocate::operator new(size);
}

//---------------------------------------------------------------------------

bool PatternComponent::CallNext(const char *p, PatternContext &context) const {
  return next->Match(p, context);
}

void PatternComponent::RemoveEpsilon() {
  while (next->IsEpsilon()) {
    next = next->next;
  }
  next->RemoveEpsilon();
}

void PatternComponent::UpdateQuickReject(
    PatternQuickReject &quickReject) const {
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
    if (*p++ != *compareP++) {
      return false;
    }
  }

  return CallNext(p, context);
}

bool CharacterSetPatternComponent::Match(const char *p,
                                         PatternContext &context) const {
  const uint8_t c = *(uint8_t *)p;
  if (c >= 128) {
    return false;
  }
  const unsigned int index = c / 8;
  const unsigned int bit = 1 << (c & 7);
  if ((mask[index] & bit) == 0) {
    return false;
  }

  return CallNext(p + 1, context);
}

bool CapturePatternComponent::Match(const char *p,
                                    PatternContext &context) const {

  const char **const capture = &context.captureList[index];
  const char *previous = *capture;
  *capture = p;
  const bool result = CallNext(p, context);
  if (!result) {
    *capture = previous;
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

  while (branch->IsEpsilon()) {
    branch = branch->next;
  }
  branch->RemoveEpsilon();

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
#if JAVELIN_USE_PATTERN_JIT
  components =
      (PatternComponent **)operator new(4 * sizeof(PatternComponent *));
#else
  components = (PatternComponent **)malloc(4 * sizeof(PatternComponent *));
#endif
  components[0] = initialComponent;
}

void ContainerPatternComponent::Add(PatternComponent *component) {
  const size_t capacity = (componentCount + 3) & -4;
  if (componentCount == capacity) {
#if JAVELIN_USE_PATTERN_JIT
    PatternComponent **newComponents = (PatternComponent **)operator new(
        (capacity + 4) * sizeof(PatternComponent *));
    memcpy(newComponents, components,
           (capacity + 4) * sizeof(PatternComponent *));
    components = newComponents;
#else
    components = (PatternComponent **)realloc(
        components, (capacity + 4) * sizeof(PatternComponent *));
#endif
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
