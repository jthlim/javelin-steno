//---------------------------------------------------------------------------

#pragma once
#include "pattern_quick_reject.h"
#include "str.h"
#include <stddef.h>

//---------------------------------------------------------------------------

class PatternComponent;
struct PatternContext;

//---------------------------------------------------------------------------

struct PatternMatch {
  bool match;
  const char *captures[8];

  // This is input, but stored here for quick access.
  const char *end;

  char *Replace(const char *format) const;
  void SetCapture(size_t n, const char *p) {
    captures[2 * n] = p;
    captures[2 * n + 1] = p + Str::Length(p);
  }

  friend class Pattern;
};

// Super simple regex implementation tailored to embedded steno.
// This whole thing compiles to less than 4kb.
//
// Many typical regex library things are missing here, since they're just
// not needed. And the approaches chosen are NOT appropriate for more
// general regex libraries.
//
// But it *DOES* support group captures, which is the key thing missing
// from other tiny regex libraries.
//
// Notes:
//
// * It relies on tail call optimizations kicking in to avoid excessive
//   stack usage.
//
// * There isn't even proper cleanup here, because it won't be used.
class Pattern {
public:
  static Pattern Compile(const char *pattern);

  PatternMatch Match(const char *text, size_t length) const;
  PatternMatch Match(const char *text) const {
    return Match(text, Str::Length(text));
  }
  PatternMatch MatchBypassingQuickReject(const char *text, size_t length) const;
  PatternMatch Search(const char *text, size_t length) const;

  bool IsPossibleMatch(PatternQuickReject inputQuickReject) const {
    return inputQuickReject.IsPossibleMatch(quickReject);
  }

#if RUN_TESTS
  bool HasEndAnchor() const;
  size_t GetMinimumLength() const;
  size_t GetMaximumLength() const;
#endif

  const PatternQuickReject &GetQuickReject() const { return quickReject; }

private:
#if JAVELIN_USE_PATTERN_JIT
  Pattern(bool (*matchMethod)(const char *start, const char **captures,
                              const char *text),
          size_t minimumLength, PatternQuickReject quickReject)
      : matchMethod(matchMethod), minimumLength(minimumLength),
        quickReject(quickReject) {}

  bool (*matchMethod)(const char *start, const char **captures,
                      const char *text);

#else
  Pattern(PatternComponent *root, size_t minimumLength,
          PatternQuickReject quickReject)
      : root(root), minimumLength(minimumLength), quickReject(quickReject) {}

  PatternComponent *root;
#endif

  size_t minimumLength;
  PatternQuickReject quickReject;

  struct BuildContext;
  struct BuildResult;
  struct BuildAtomResult;

  static BuildResult ParseAlternate(BuildContext &c);
  static BuildResult ParseSequence(BuildContext &c);
  static BuildResult ParseQuantifiedAtom(BuildContext &c);
  static BuildAtomResult ParseAtom(BuildContext &c);
  static BuildResult ParseQuantifier(BuildContext &c, const BuildResult &atom);

  static const char *FindLiteralEnd(const char *p);
};

//---------------------------------------------------------------------------
