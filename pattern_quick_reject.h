//---------------------------------------------------------------------------

#include <stdint.h>

//---------------------------------------------------------------------------

class PatternQuickReject {
public:
  PatternQuickReject() = default;
  PatternQuickReject(const char *p) { Update(p); }

  void Update(int c) {
#if JAVELIN_ASSEMBLER_THUMB2
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

  bool IsPossibleMatch(const PatternQuickReject patternQuickReject) {
    return (patternQuickReject.mask & ~mask) == 0;
  }

private:
  uint32_t mask = 0;

  static void Update(int c, uint32_t &mask) {
#if JAVELIN_ASSEMBLER_THUMB2
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
