//---------------------------------------------------------------------------

#include "pattern.h"

#include <assert.h>

#include "pattern_component.h"
#include "str.h"

//---------------------------------------------------------------------------

struct Pattern::BuildContext {
  const char *p;
  int captureIndex;
};

struct Pattern::BuildResult {
  BuildResult(PatternComponent *p) : head(p), tail(p) {}
  BuildResult(PatternComponent *pHead, PatternComponent *pTail)
      : head(pHead), tail(pTail) {}

  PatternComponent *head;
  PatternComponent *tail;
};

//---------------------------------------------------------------------------

Pattern Pattern::Compile(const char *p) {
  BuildContext context;
  context.p = p;
  context.captureIndex = 2;

  PatternComponent *captureStart = new CapturePatternComponent(0);
  BuildResult result = ParseAlternate(context);
  captureStart->next = result.head;
  result.tail->next = new CapturePatternComponent(1);

  // If this assert is hit, then the entire pattern hasn't been processed.
  assert(*context.p == '\0');
  captureStart->RemoveEpsilon();

  return Pattern(captureStart);
}

//---------------------------------------------------------------------------

Pattern::BuildResult Pattern::ParseAlternate(BuildContext &c) {
  BuildResult result = ParseSequence(c);
  assert(result.head != nullptr);
  assert(result.tail != nullptr);
  if (*c.p != '|') {
    return result;
  }

  AlternatePatternComponent *alternate =
      new AlternatePatternComponent(result.head);
  EpsilonPatternComponent *epsilon = new EpsilonPatternComponent;

  result.tail->next = epsilon;

  while (*c.p == '|') {
    c.p++;
    BuildResult result = ParseSequence(c);
    assert(result.head != nullptr);
    assert(result.tail != nullptr);
    alternate->Add(result.head);
    result.tail->next = epsilon;
  }

  return BuildResult(alternate, epsilon);
}

Pattern::BuildResult Pattern::ParseSequence(BuildContext &c) {
  BuildResult result = ParseQuantifiedAtom(c);

  for (;;) {
    BuildResult nextElement = ParseQuantifiedAtom(c);
    if (nextElement.head == nullptr) {
      return result;
    }

    result.tail->next = nextElement.head;
    result.tail = nextElement.tail;
  }

  return result;
}

Pattern::BuildResult Pattern::ParseQuantifiedAtom(BuildContext &c) {
  if (c.p[0] == '.') {
    if (c.p[1] == '*') {
      c.p += 2;
      return BuildResult(new AnyStarPatternComponent);
    } else if (c.p[1] == '+') {
      c.p += 2;
      PatternComponent *any = new AnyPatternComponent;
      PatternComponent *anyStar = new AnyStarPatternComponent;
      any->next = anyStar;
      return BuildResult(any, anyStar);
    }
  }

  BuildResult atom = ParseAtom(c);
  if (atom.head == nullptr) {
    return atom;
  }

  return ParseQuantifier(c, atom);
}

Pattern::BuildResult Pattern::ParseAtom(BuildContext &c) {
  switch (*c.p) {
  case '\0':
  case ')':
  case '|':
    return BuildResult(nullptr);
  case '^':
    c.p++;
    return BuildResult(new StartOfLinePatternComponent);

  case '$':
    c.p++;
    return BuildResult(new EndOfLinePatternComponent);

  case '(': {
    c.p++;

    if (c.p[0] == '?' && c.p[1] == ':') {
      c.p += 2;
      BuildResult component = ParseAlternate(c);
      assert(*c.p == ')');
      c.p++;
      return component;
    }

    assert(c.captureIndex < 8);

    int captureIndex = c.captureIndex;
    c.captureIndex += 2;
    PatternComponent *captureStart = new CapturePatternComponent(captureIndex);
    BuildResult component = ParseAlternate(c);
    assert(*c.p == ')');
    c.p++;
    captureStart->next = component.head;
    PatternComponent *captureEnd =
        new CapturePatternComponent(captureIndex + 1);
    component.tail->next = captureEnd;
    return BuildResult(captureStart, captureEnd);
  }
  case '\\': {
    c.p++;
    switch (*c.p) {
    case '^':
    case '\\':
      return BuildResult(new LiteralPatternComponent(Str::DupN(c.p++, 1)));
    default:
      assert(!"Unhandled symbol");
    }
  }
  case '[': {
    CharacterSetComponent *component = new CharacterSetComponent();
    const char *p = c.p + 1;
    while (*p != ']') {
      assert(*p);

      if (p[0] == '-' && p[1] != ']') {
        for (int index = p[-1]; index <= p[1]; ++index) {
          component->SetBit(index);
        }
      } else {
        component->SetBit(*p);
      }
      ++p;
    }
    c.p = p + 1;

    return BuildResult(component);
  }
  case '.':
    c.p++;
    return BuildResult(new AnyPatternComponent);

  default:
    const char *pStart = c.p;
    c.p = FindLiteralEnd(pStart);
    return BuildResult(
        new LiteralPatternComponent(Str::DupN(pStart, c.p - pStart)));
  }
}

const char *Pattern::FindLiteralEnd(const char *p) {
  const char *pStart = p;

  p++;
  for (;;) {
    switch (*p) {
    case '\0':
    case ')':
    case '|':
    case '^':
    case '$':
    case '(':
    case '\\':
    case '[':
    case '.':
      return p;

    case '*':
    case '+':
    case '?':
      return (p - 1 == pStart) ? p : p - 1;

    default:
      ++p;
    }
  }
}

Pattern::BuildResult Pattern::ParseQuantifier(BuildContext &c,
                                              BuildResult atom) {
  switch (*c.p) {
  case '*': {
    c.p++;
    BranchPatternComponent *star = new BranchPatternComponent(atom.head);
    atom.tail->next = star;

    return BuildResult(star);
  }
  case '+': {
    c.p++;
    BranchPatternComponent *plus = new BranchPatternComponent(atom.head);
    atom.tail->next = plus;

    return BuildResult(atom.head, plus);
  }

  case '?': {
    c.p++;
    PatternComponent *epsilon = new EpsilonPatternComponent;
    BranchPatternComponent *alternate = new BranchPatternComponent(atom.head);
    alternate->next = epsilon;
    atom.tail->next = epsilon;
    return BuildResult(alternate, epsilon);
  }
  default:
    return atom;
  }
}

//---------------------------------------------------------------------------

PatternMatch Pattern::Match(const char *text) const {
  struct PatternMatch result;
  PatternContext context = {
      .start = text,
      .captureList = result.captures,
  };
  result.match = root->Match(text, context);
  return result;
}

PatternMatch Pattern::Search(const char *text) const {
  struct PatternMatch result;
  PatternContext context = {
      .start = text,
      .captureList = result.captures,
  };
  do {
    result.match = root->Match(text, context);
  } while (!result.match && *text++ != '\0');
  return result;
}

char *Pattern::Replace(char *text, const char *templ) const {
  PatternMatch match = Search(text);
  if (!match.match) {
    return text;
  }
  char *prefix = Str::DupN(text, match.captures[0] - text);
  char *suffix =
      Str::DupN(match.captures[1], text + strlen(text) - match.captures[1]);
  char *replacement = match.Replace(templ);

  char *result = Str::Asprintf("%s%s%s", prefix, replacement, suffix);
  free(prefix);
  free(suffix);
  free(replacement);
  free(text);
  return result;
}

//---------------------------------------------------------------------------

char *PatternMatch::Replace(const char *s) const {
  assert(match);

  char *buffer = (char *)malloc(256);
  char *d = buffer;
  for (;;) {
    switch (*s) {
    case '\0':
      *d++ = '\0';
      assert(d <= buffer + 256);
      return (char *)realloc(buffer, d - buffer);

    case '\\': {
      ++s;
      int index = *s - '0';
      ++s;
      assert(0 <= index && index < 4);
      const char *start = captures[index * 2];
      const char *end = captures[index * 2 + 1];
      memcpy(d, start, end - start);
      d += end - start;
      break;
    }
    default:
      *d++ = *s++;
      break;
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"
#include <stdio.h>

// spellchecker: disable
TEST_BEGIN("Pattern: Simple test") {
  const Pattern pattern = Pattern::Compile("a(b|c)d");
  const char *abd = "abd";
  PatternMatch match = pattern.Match(abd);
  assert(match.match);
  assert(match.captures[0] == abd);
  assert(match.captures[1] == abd + 3);
  assert(match.captures[2] == abd + 1);
  assert(match.captures[3] == abd + 2);
}
TEST_END

TEST_BEGIN("Pattern: Repeat test") {
  const Pattern pattern = Pattern::Compile("a*d");
  assert(pattern.Match("d").match);
  assert(pattern.Match("ad").match);
  assert(pattern.Match("aad").match);
}
TEST_END

TEST_BEGIN("Pattern: PlusRepeat pattern test") {
  const Pattern pattern = Pattern::Compile("a+bd");
  assert(pattern.Match("bd").match == false);
  assert(pattern.Match("abd").match);
  assert(pattern.Match("aabd").match);
}
TEST_END

TEST_BEGIN("Pattern: Replace tests") {
  const Pattern pattern = Pattern::Compile("(a*)b");
  char *t1 = pattern.Match("aaaab").Replace("\\0\\1");
  assert(strcmp(t1, "aaaabaaaa") == 0);
  free(t1);

  char *t2 = pattern.Match("b").Replace("\\0\\1");
  assert(strcmp(t2, "b") == 0);
  free(t2);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example test") {
  const Pattern pattern = Pattern::Compile(
      R"(^(.*(?:[bcdfghjklmnprstvwxyz]|qu)[aeiou])([bcdfgklmnprtvz]) \^ ([aeiouy].*)$)");

  char *t1 = pattern.Match("defer ^ ed").Replace(R"(\1\2\2\3)");
  assert(strcmp(t1, "deferred") == 0);
  free(t1);
}
TEST_END
// spellchecker: enable

//---------------------------------------------------------------------------
