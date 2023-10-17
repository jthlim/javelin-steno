//---------------------------------------------------------------------------

#include "pattern_component.h"
#include "pattern_quick_reject.h"
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

void PatternComponent::UpdateRanges(PatternRangeContext &context) {
  if (!next) {
    return;
  }
  next->UpdateRanges(context);
}

void PatternComponent::UpdateRangesForSingleByte(PatternRangeContext &context) {
  PatternComponent::UpdateRanges(context);
  context.AddLength(1);
}

//---------------------------------------------------------------------------

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

void AnyPatternComponent::UpdateRanges(PatternRangeContext &context) {
  UpdateRangesForSingleByte(context);
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
  if (maximumMatchLength) {
    const char *maximumMatchStart = p - maximumMatchLength;
    if (maximumMatchStart >= start) {
      start = maximumMatchStart;
    }
  }
  p -= minimumMatchLength;

  while (p >= start) {
    if (localNext->Match(p, context)) {
      return true;
    }
    --p;
  }
  return false;
}

void AnyStarPatternComponent::UpdateRanges(PatternRangeContext &context) {
  PatternComponent::UpdateRanges(context);
  minimumMatchLength = context.minimumLength;
  if (context.hasAnchor && context.HasMaximumLength()) {
    maximumMatchLength = context.maximumLength;
  }
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

void BackReferencePatternComponent::UpdateRanges(PatternRangeContext &context) {
  PatternComponent::UpdateRanges(context);
  context.SetUnlimitedMaximumLength();
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

void CharacterSetComponent::UpdateRanges(PatternRangeContext &context) {
  UpdateRangesForSingleByte(context);
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

void BranchPatternComponent::UpdateRanges(PatternRangeContext &context) {
  if (context.visited.Contains(this)) {
    context.SetUnlimitedMaximumLength();
  } else {
    context.visited.Add(this);
    PatternComponent::UpdateRanges(context);
  }
}

bool StartOfLinePatternComponent::Match(const char *p,
                                        PatternContext &context) const {
  return (p == context.start) && CallNext(p, context);
}

bool EndOfLinePatternComponent::Match(const char *p,
                                      PatternContext &context) const {
  return (*p == '\0') && CallNext(p, context);
}

void EndOfLinePatternComponent::UpdateRanges(PatternRangeContext &context) {
  PatternComponent::UpdateRanges(context);
  context.hasAnchor = true;
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

void BytePatternComponent::UpdateRanges(PatternRangeContext &context) {
  UpdateRangesForSingleByte(context);
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

void LiteralPatternComponent::UpdateRanges(PatternRangeContext &context) {
  PatternComponent::UpdateRanges(context);
  context.AddLength(strlen(text));
}

//---------------------------------------------------------------------------

ContainerPatternComponent::ContainerPatternComponent(
    PatternComponent *initialComponent) {
  componentCount = 1;
  components = new PatternComponent *[4];
  components[0] = initialComponent;
}

void ContainerPatternComponent::Add(PatternComponent *component) {
  size_t capacity = (componentCount + 3) & -4;
  if (componentCount == capacity) {
    PatternComponent **newComponents = new PatternComponent *[capacity + 4];

    memcpy(newComponents, components,
           capacity * sizeof(PatternComponent *)); // NOLINT

    delete[] components;
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

void AlternatePatternComponent::UpdateRanges(PatternRangeContext &context) {
  size_t minimumLength = (size_t)-1;
  size_t maximumLength = 0;
  bool hasAnchor = true;

  for (size_t i = 0; i < componentCount; ++i) {
    PatternRangeContext localContext;
    components[i]->UpdateRanges(localContext);

    if (minimumLength > localContext.minimumLength) {
      minimumLength = localContext.minimumLength;
    }
    if (maximumLength < localContext.maximumLength) {
      maximumLength = localContext.maximumLength;
    }
    if (!localContext.hasAnchor) {
      hasAnchor = false;
    }
  }

  context.hasAnchor = hasAnchor;
  context.minimumLength = minimumLength;
  context.maximumLength = maximumLength;
}

//---------------------------------------------------------------------------
