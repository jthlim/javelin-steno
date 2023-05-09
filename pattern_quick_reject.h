//---------------------------------------------------------------------------

#include <stdint.h>

//---------------------------------------------------------------------------

class PatternQuickReject {
public:
  PatternQuickReject() = default;
  PatternQuickReject(const char *p) { Update(p); }

  void Update(int c) {
#if JAVELIN_CPU_CORTEX_M0 || JAVELIN_CPU_CORTEX_M4
    // On arm, shifting by more than the width results in 0.
    mask |= 1 << (c - 'a');
#else
    // On x86, shifting only uses the lowest bits.
    if ('a' <= c && c <= 'z') {
      mask |= 1 << (c - 'a');
    }
#endif
  }

  void Update(const char *p) {
    while (*p) {
      Update(*p++);
    }
  }

  bool IsPossibleMatch(const PatternQuickReject patternQuickReject) const {
    return (patternQuickReject.mask & ~mask) == 0;
  }

private:
  uint32_t mask = 0;

  static void Update(int c, uint32_t &mask) {
#if JAVELIN_CPU_CORTEX_M0 || JAVELIN_CPU_CORTEX_M4
    // On arm, shifting by more than the width results in 0.
    mask |= ~(1 << (c - 'a'));
#else
    // On x86, shifting only uses the lowest bits.
    if ('a' <= c && c <= 'z') {
      mask |= ~(1 << (c - 'a'));
    }
#endif
  }
};

//---------------------------------------------------------------------------
