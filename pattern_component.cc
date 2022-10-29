//---------------------------------------------------------------------------

#include "pattern_component.h"
#include <string.h>

//---------------------------------------------------------------------------

bool PatternComponent::CallNext(const char *p, PatternContext &context) {
  return !next || next->Match(p, context);
}

bool EpsilonPatternComponent::Match(const char *p, PatternContext &context) {
  return CallNext(p, context);
}

bool AnyPatternComponent::Match(const char *p, PatternContext &context) {
  if (*p == '\0') {
    return false;
  }
  return CallNext(p + 1, context);
}

bool AnyStarPatternComponent::Match(const char *p, PatternContext &context) {
  const char *start = p;
  while (*p) {
    ++p;
  }
  while (p >= start) {
    if (CallNext(p, context)) {
      return true;
    }
    --p;
  }
  return false;
}

bool CharacterSetComponent::Match(const char *p, PatternContext &context) {
  unsigned int c = *(uint8_t *)p;
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

bool CapturePatternComponent::Match(const char *p, PatternContext &context) {
  bool result = CallNext(p, context);
  if (result) {
    // Since no backreferences are required, this can just be set here,
    // instead of before the call and restoring it if there's no match.
    context.captureList[index] = p;
  }
  return result;
}

bool BranchPatternComponent::Match(const char *p, PatternContext &context) {
  return branch->Match(p, context) || CallNext(p, context);
}

bool StartOfLinePatternComponent::Match(const char *p,
                                        PatternContext &context) {
  return (p == context.start) && CallNext(p, context);
}

bool EndOfLinePatternComponent::Match(const char *p, PatternContext &context) {
  return (*p == '\0') && CallNext(p, context);
}

//---------------------------------------------------------------------------

bool LiteralPatternComponent::Match(const char *p, PatternContext &context) {
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
  components = new PatternComponent *[4];
  components[0] = initialComponent;
}

void ContainerPatternComponent::Add(PatternComponent *component) {
  size_t capacity = (componentCount + 3) & -4;
  if (componentCount == capacity) {
    PatternComponent **newComponents = new PatternComponent *[capacity + 4];

    // Better to use a memcpy here, but clang starts warning
    for (size_t i = 0; i < capacity; ++i) {
      newComponents[i] = components[i];
    }

    delete[] components;
    components = newComponents;
  }
  components[componentCount++] = component;
}

//---------------------------------------------------------------------------

bool AlternatePatternComponent::Match(const char *p, PatternContext &context) {
  for (size_t i = 0; i < componentCount; ++i) {
    if (components[i]->Match(p, context)) {
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------
