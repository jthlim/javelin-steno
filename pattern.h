//---------------------------------------------------------------------------

#pragma once
#include "pattern_quick_reject.h"
#include <stddef.h>

//---------------------------------------------------------------------------

class PatternComponent;

//---------------------------------------------------------------------------

struct PatternMatch {
  bool match;
  const char *captures[8];

  char *Replace(const char *format) const;

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

  PatternMatch Match(const char *text) const;
  PatternMatch MatchBypassingQuickReject(const char *text) const;
  PatternMatch Search(const char *text) const;

  // Will free text if there's a replacement and return a new string.
  char *Replace(char *text, const char *format) const;

  bool IsPossibleMatch(PatternQuickReject inputQuickReject) const {
    return inputQuickReject.IsPossibleMatch(quickReject);
  }

private:
  Pattern(PatternComponent *root, PatternQuickReject quickReject)
      : root(root), quickReject(quickReject) {}

  PatternComponent *root;
  PatternQuickReject quickReject;

  struct BuildContext;
  struct BuildResult;

  static BuildResult ParseAlternate(BuildContext &c);
  static BuildResult ParseSequence(BuildContext &c);
  static BuildResult ParseQuantifiedAtom(BuildContext &c);
  static BuildResult ParseAtom(BuildContext &c);
  static BuildResult ParseQuantifier(BuildContext &c, BuildResult atom);

  static const char *FindLiteralEnd(const char *p);
};

//---------------------------------------------------------------------------
