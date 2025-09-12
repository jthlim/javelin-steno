//---------------------------------------------------------------------------

#include "pattern_component.h"
#include "pattern_quick_reject.h"
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

SuccessPatternComponent SuccessPatternComponent::instance;
size_t PatternRecurseContext::currentReference;

//---------------------------------------------------------------------------

void *PatternComponent::operator new(size_t size) {
  return PoolAllocate::operator new(size);
}

//---------------------------------------------------------------------------

bool PatternComponent::CallNext(const char *p, PatternContext &context) const {
  return next->Match(p, context);
}

void PatternComponent::GenerateMetrics(const PatternRecurseContext &context) {
  next->GenerateMetrics(context);
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

bool PatternComponent::HasEndAnchor(
    const PatternRecurseContext &context) const {
  return next->HasEndAnchor(context);
}

size_t
PatternComponent::GetMinimumLength(const PatternRecurseContext &context) const {
  return next->GetMinimumLength(context);
}

size_t
PatternComponent::GetMaximumLength(const PatternRecurseContext &context) const {
  return next->GetMaximumLength(context);
}

//---------------------------------------------------------------------------

size_t SingleBytePatternComponent::GetMinimumLength(
    const PatternRecurseContext &context) const {
  return 1 + super::GetMinimumLength(context);
}
size_t SingleBytePatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  const size_t nextLength = super::GetMaximumLength(context);
  if (nextLength == INFINITE_LENGTH) {
    return nextLength;
  }
  return 1 + nextLength;
}

//---------------------------------------------------------------------------

bool EpsilonPatternComponent::Match(const char *p,
                                    PatternContext &context) const {
  return CallNext(p, context);
}

void EpsilonPatternComponent::GenerateMetrics(
    const PatternRecurseContext &context) {
  if (generateMetricsContext == context) {
    return;
  }
  generateMetricsContext = context;
  super::GenerateMetrics(context);
}

bool EpsilonPatternComponent::HasEndAnchor(
    const PatternRecurseContext &context) const {
  if (recurseContext != context) {
    recurseData.hasEndAnchor = super::HasEndAnchor(context);
    recurseContext = context;
  }
  return recurseData.hasEndAnchor;
}

size_t EpsilonPatternComponent::GetMinimumLength(
    const PatternRecurseContext &context) const {
  if (recurseContext != context) {
    recurseData.length = super::GetMinimumLength(context);
    recurseContext = context;
  }
  return recurseData.length;
}

size_t EpsilonPatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  if (recurseContext != context) {
    recurseData.length = super::GetMaximumLength(context);
    recurseContext = context;
  }
  return recurseData.length;
}

//---------------------------------------------------------------------------

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
  p = context.captures[8]; // p = end

  if (hasEndAnchor) {
    if (maximumLength != INFINITE_LENGTH) {
      const char *startWithMaximumLength = p - maximumLength;
      if (start < startWithMaximumLength) {
        start = startWithMaximumLength;
      }
    }
    p -= minimumLength;
  }

  while (p >= start) {
    if (localNext->Match(p, context)) {
      return true;
    }
    --p;
  }
  return false;
}

void AnyStarPatternComponent::GenerateMetrics(
    const PatternRecurseContext &context) {
  if (hasProcessed) {
    return;
  }
  hasProcessed = true;

  PatternRecurseContext dataContext;
  hasEndAnchor = super::HasEndAnchor(dataContext);
  if (!hasEndAnchor) {
    minimumLength = 0;
    maximumLength = INFINITE_LENGTH;
    return;
  }
  dataContext.Reset();
  minimumLength = super::GetMinimumLength(dataContext);
  dataContext.Reset();
  maximumLength = super::GetMaximumLength(dataContext);

  super::GenerateMetrics(context);
}

size_t AnyStarPatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  return INFINITE_LENGTH;
}

bool BackReferencePatternComponent::Match(const char *p,
                                          PatternContext &context) const {
  const char *compareP = context.captures[index * 2];
  const char *comparedEnd = context.captures[index * 2 + 1];

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

  const char **const capture = &context.captures[index];
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
  processed = false;
}

void BranchPatternComponent::GenerateMetrics(
    const PatternRecurseContext &context) {
  if (processed) {
    return;
  }
  processed = true;

  branch->GenerateMetrics(context);
  PatternComponent::GenerateMetrics(context);
  processed = false;
}

bool BranchPatternComponent::HasEndAnchor(
    const PatternRecurseContext &context) const {
  switch (type) {
  case BranchType::BRANCH_BACK:
  case BranchType::NEXT_FORWARD:
    return next->HasEndAnchor(context);
  case BranchType::NEXT_BACK:
  case BranchType::BRANCH_FORWARD:
    return branch->HasEndAnchor(context);
  default:
    __builtin_unreachable();
  }
}
size_t BranchPatternComponent::GetMinimumLength(
    const PatternRecurseContext &context) const {
  switch (type) {
  case BranchType::BRANCH_BACK:
  case BranchType::NEXT_FORWARD:
    return next->GetMinimumLength(context);
  case BranchType::BRANCH_FORWARD:
  case BranchType::NEXT_BACK:
    return branch->GetMinimumLength(context);
  default:
    __builtin_unreachable();
  }
}
size_t BranchPatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  switch (type) {
  case BranchType::BRANCH_BACK:
  case BranchType::NEXT_BACK:
    return INFINITE_LENGTH;
  case BranchType::BRANCH_FORWARD:
    return branch->GetMaximumLength(context);
  case BranchType::NEXT_FORWARD:
    return next->GetMaximumLength(context);
  default:
    __builtin_unreachable();
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

size_t LiteralPatternComponent::GetMinimumLength(
    const PatternRecurseContext &context) const {
  return strlen(text) + super::GetMinimumLength(context);
}
size_t LiteralPatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  const size_t nextLength = super::GetMaximumLength(context);
  if (nextLength == INFINITE_LENGTH) {
    return nextLength;
  }
  return strlen(text) + nextLength;
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

void AlternatePatternComponent::GenerateMetrics(
    const PatternRecurseContext &context) {
  for (size_t i = 0; i < componentCount; ++i) {
    components[i]->GenerateMetrics(context);
  }
}

bool AlternatePatternComponent::HasEndAnchor(
    const PatternRecurseContext &context) const {
  for (size_t i = 0; i < componentCount; ++i) {
    if (!components[i]->HasEndAnchor(context)) {
      return false;
    }
  }
  return true;
}

size_t AlternatePatternComponent::GetMinimumLength(
    const PatternRecurseContext &context) const {
  size_t result = INFINITE_LENGTH;
  for (size_t i = 0; i < componentCount; ++i) {
    const size_t length = components[i]->GetMinimumLength(context);
    if (length < result) {
      result = length;
    }
  }
  return result;
}

size_t AlternatePatternComponent::GetMaximumLength(
    const PatternRecurseContext &context) const {
  size_t result = 0;
  for (size_t i = 0; i < componentCount; ++i) {
    const size_t length = components[i]->GetMaximumLength(context);
    if (length > result) {
      result = length;
    }
  }
  return result;
}

//---------------------------------------------------------------------------
