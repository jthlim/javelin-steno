//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class PatternComponent;

//---------------------------------------------------------------------------

struct PatternMatch {
  bool match;
  const char *captures[8];

  char *Replace(const char *s) const;

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
  PatternMatch Search(const char *text) const;

  // Will free text if there's a replacement and return a new string.
  char *Replace(char *text, const char *templ) const;

private:
  Pattern(PatternComponent *root, uint32_t accelerator)
      : root(root), accelerator(accelerator) {}

  PatternComponent *root;

  // This is a bit field of characters a-z that must be present in the text.
  uint32_t accelerator;

  struct BuildContext;
  struct BuildResult;

  bool EarlyReject(const char *text) const;

  static BuildResult ParseAlternate(BuildContext &c);
  static BuildResult ParseSequence(BuildContext &c);
  static BuildResult ParseQuantifiedAtom(BuildContext &c);
  static BuildResult ParseAtom(BuildContext &c);
  static BuildResult ParseQuantifier(BuildContext &c, BuildResult atom);

  static const char *FindLiteralEnd(const char *p);
};

//---------------------------------------------------------------------------
