//---------------------------------------------------------------------------

#include "pattern.h"
#include "pattern_component.h"
#include "str.h"
#include <assert.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct Pattern::BuildContext {
  const char *p;
  size_t captureIndex;
  CapturePatternComponent *captures[8];

  void SetCaptureComponents(size_t index, CapturePatternComponent *start,
                            CapturePatternComponent *end) {
    captures[index] = start;
    captures[index + 1] = end;
  }
};

struct Pattern::BuildResult {
  BuildResult(PatternComponent *p) : head(p), tail(p) {}
  BuildResult(PatternComponent *pHead, PatternComponent *pTail)
      : head(pHead), tail(pTail) {}

  PatternComponent *head;
  PatternComponent *tail;
};

struct Pattern::BuildAtomResult : public Pattern::BuildResult {
  using super = Pattern::BuildResult;

  BuildAtomResult(const BuildResult &b, bool isQuantified)
      : super(b), isQuantified(isQuantified) {}
  BuildAtomResult(PatternComponent *pHead, PatternComponent *pTail,
                  bool isQuantified)
      : super(pHead, pTail), isQuantified(isQuantified) {}

  const bool isQuantified;
};

//---------------------------------------------------------------------------

Pattern Pattern::Compile(const char *p) {
  BuildContext context;
  context.p = p;
  context.captureIndex = 2;

  CapturePatternComponent *captureStart = new CapturePatternComponent(0);
  const BuildResult result = ParseAlternate(context);
  captureStart->next = result.head;
  CapturePatternComponent *captureEnd = new CapturePatternComponent(1);
  result.tail->next = captureEnd;
  context.SetCaptureComponents(0, captureStart, captureEnd);

  // If this assert is hit, then the entire pattern hasn't been processed.
  assert(*context.p == '\0');

  PatternRecurseContext generateMetricsContext;
  captureStart->GenerateMetrics(generateMetricsContext);

  PatternRecurseContext minimumLengthContext;

  const size_t minimumLength =
      captureStart->GetMinimumLength(minimumLengthContext);

  captureStart->MarkRequiredCaptures(nullptr);
#if !JAVELIN_USE_PATTERN_JIT
  captureStart->RemoveEpsilon();
#endif

  PatternQuickReject quickReject;
  captureStart->UpdateQuickReject(quickReject);

#if JAVELIN_USE_PATTERN_JIT
  PatternJitContext jitContext;
  captureStart->Compile(jitContext);
  bool (*matchMethod)(const char *, const char **, const char *) =
      jitContext.Build();
  PatternComponent::ResetPoolAllocator();
  return Pattern(matchMethod, minimumLength, quickReject);
#else
  return Pattern(captureStart, minimumLength, quickReject);
#endif
}

//---------------------------------------------------------------------------

#if RUN_TESTS

bool Pattern::HasEndAnchor() const {
  PatternRecurseContext context;
  return root->HasEndAnchor(context);
}

size_t Pattern::GetMinimumLength() const {
  PatternRecurseContext context;
  return root->GetMinimumLength(context);
}

size_t Pattern::GetMaximumLength() const {
  PatternRecurseContext context;
  return root->GetMaximumLength(context);
}

#endif

//---------------------------------------------------------------------------

Pattern::BuildResult Pattern::ParseAlternate(BuildContext &c) {
  const BuildResult result = ParseSequence(c);
  assert(result.head != nullptr);
  assert(result.tail != nullptr);
  if (*c.p != '|') {
    return result;
  }

  AlternatePatternComponent *alternate =
      new AlternatePatternComponent(result.head);
  EpsilonPatternComponent *epsilon = new EpsilonPatternComponent;

  alternate->next = epsilon;
  result.tail->next = epsilon;

  while (*c.p == '|') {
    c.p++;
    const BuildResult result = ParseSequence(c);
    assert(result.head != nullptr);
    assert(result.tail != nullptr);
    alternate->Add(result.head);
    result.tail->next = alternate->next;
  }

  return BuildResult(alternate, alternate->next);
}

Pattern::BuildResult Pattern::ParseSequence(BuildContext &c) {
  BuildResult result = ParseQuantifiedAtom(c);
  if (result.head == nullptr) {
    return BuildResult(&SuccessPatternComponent::instance);
  }

  for (;;) {
    const BuildResult nextElement = ParseQuantifiedAtom(c);
    if (nextElement.head == nullptr) {
      return result;
    }

    result.tail->next = nextElement.head;
    result.tail = nextElement.tail;
  }

  return result;
}

Pattern::BuildResult Pattern::ParseQuantifiedAtom(BuildContext &c) {
  const BuildAtomResult atom = ParseAtom(c);
  if (atom.head == nullptr || atom.isQuantified) {
    return atom;
  }

  return ParseQuantifier(c, atom);
}

Pattern::BuildAtomResult Pattern::ParseAtom(BuildContext &c) {
  switch (*c.p) {
  case '\0':
  case ')':
  case '|':
    return BuildAtomResult(nullptr, false);
  case '^':
    c.p++;
    return BuildAtomResult(new StartOfLinePatternComponent, false);

  case '$':
    c.p++;
    return BuildAtomResult(new EndOfLinePatternComponent, false);

  case '(': {
    c.p++;

    if (c.p[0] == '?' && c.p[1] == ':') {
      c.p += 2;
      BuildResult component = ParseAlternate(c);
      assert(*c.p == ')');
      c.p++;
      return BuildAtomResult(component, false);
    }

    assert(c.captureIndex < 8);

    const size_t captureIndex = c.captureIndex;
    c.captureIndex += 2;
    CapturePatternComponent *captureStart =
        new CapturePatternComponent(captureIndex);
    const BuildResult component = ParseAlternate(c);
    assert(*c.p == ')');
    c.p++;

    CapturePatternComponent *captureEnd =
        new CapturePatternComponent(captureIndex + 1);
    c.SetCaptureComponents(captureIndex, captureStart, captureEnd);

    if (*c.p == '?') {
      c.p++;
      // Special case (x)? to become ((?:x)?) to enable the more performant
      // AlwaysCapture more often. This is not appropriate for a general regex
      // engine, where it may be necessary to test if an empty group has been
      // matched, but for a steno regex engine, this is not a concern.
      PatternComponent *epsilon = new EpsilonPatternComponent;
      BranchPatternComponent *alternate =
          new BranchPatternComponent(component.head, BranchType::NEXT_FORWARD);
      alternate->next = epsilon;
      component.tail->next = epsilon;
      captureStart->next = alternate;
      epsilon->next = captureEnd;
      return BuildAtomResult(captureStart, captureEnd, true);
    } else {
      captureStart->next = component.head;
      component.tail->next = captureEnd;
      return BuildAtomResult(captureStart, captureEnd, false);
    }
  }
  case '\\':
    switch (const int x = c.p[1]; x) {
    case '1':
    case '2':
    case '3': {
      const size_t index = x - '0';
      c.p += 2;
      return BuildAtomResult(
          new BackReferencePatternComponent(index, c.captures[index * 2],
                                            c.captures[index * 2 + 1]),
          false);
    }

    default:
      goto HandleLiteral;
    }
  case '[': {
    CharacterSetPatternComponent *component =
        new CharacterSetPatternComponent();
    const char *p = c.p + 1;
    bool flipBits = false;
    if (*p == '^') {
      flipBits = true;
      ++p;
    }
    if (*p == '-') {
      component->SetBit('-');
      ++p;
    }
    while (*p != ']') {
      assert(*p);

      if (p[0] == '-' && p[1] != ']') {
        for (int index = p[-1]; index <= p[1]; ++index) {
          component->SetBit(index);
        }
        // Don't increment p here -- this protects against
        // bad input where '-' is at the end of a string and there's
        // no terminating ']'.
      } else {
        component->SetBit(*p);
      }
      ++p;
    }
    c.p = p + 1;
    if (flipBits) {
      component->FlipBits();
    }

    return BuildAtomResult(component, false);
  }
  case '.':
    c.p++;
    if (*c.p == '*') {
      c.p++;
      return BuildAtomResult(new AnyStarPatternComponent, true);
    } else if (*c.p == '+') {
      c.p++;
      PatternComponent *any = new AnyPatternComponent;
      PatternComponent *anyStar = new AnyStarPatternComponent;
      any->next = anyStar;
      return BuildAtomResult(any, anyStar, true);
    }
    return BuildAtomResult(new AnyPatternComponent, false);

  default:
  HandleLiteral:
    const char *pStart = c.p;
    c.p = FindLiteralEnd(pStart);
    const size_t length = c.p - pStart;
    PatternComponent *component;
    if (length == 1) {
      component = new BytePatternComponent(*pStart);
    } else {
      component = new (length) LiteralPatternComponent(pStart, length);
    }
    return BuildAtomResult(component, false);
  }
}

const char *Pattern::FindLiteralEnd(const char *p) {
  const char *pStart = p;
  const char *previous = p;

  for (;;) {
    switch (*p) {
    case '\0':
    case ')':
    case '|':
    case '^':
    case '$':
    case '(':
    case '[':
    case '.':
      return p;

    case '\\':
      if (p[1] == '\0') {
        return p + 1;
      }
      previous = p;
      p += 2;
      break;

    case '*':
    case '+':
    case '?':
      return previous == pStart ? p : previous;

    default:
      previous = p;
      ++p;
    }
  }
}

Pattern::BuildResult Pattern::ParseQuantifier(BuildContext &c,
                                              const BuildResult &atom) {
  switch (*c.p) {
  case '*': {
    c.p++;
    BranchPatternComponent *star =
        new BranchPatternComponent(atom.head, BranchType::BRANCH_BACK);
    atom.tail->next = star;

    return BuildResult(star);
  }
  case '+': {
    c.p++;
    BranchPatternComponent *plus =
        new BranchPatternComponent(atom.head, BranchType::BRANCH_BACK);
    atom.tail->next = plus;

    return BuildResult(atom.head, plus);
  }

  case '?': {
    c.p++;
    PatternComponent *epsilon = new EpsilonPatternComponent;
    BranchPatternComponent *alternate =
        new BranchPatternComponent(atom.head, BranchType::NEXT_FORWARD);
    alternate->next = epsilon;
    atom.tail->next = epsilon;
    return BuildResult(alternate, epsilon);
  }
  default:
    return atom;
  }
}

//---------------------------------------------------------------------------

PatternMatch Pattern::Match(const char *text, size_t length) const {
  const PatternQuickReject textReject(text);
  if (!textReject.IsPossibleMatch(quickReject)) {
    PatternMatch result;
    result.match = false;
    return result;
  } else {
    return MatchBypassingQuickReject(text, length);
  }
}

PatternMatch Pattern::MatchBypassingQuickReject(const char *text,
                                                size_t length) const {
  PatternMatch result;
  if (length < minimumLength) {
    result.match = false;
  } else {
    result.end = text + length;

    // This code is in the hot path.
    // Expand this to avoid calls to __wrap_memset on rp2040.
    result.captures[2] = nullptr;
    result.captures[3] = nullptr;
    result.captures[4] = nullptr;
    result.captures[5] = nullptr;
    result.captures[6] = nullptr;
    result.captures[7] = nullptr;

#if JAVELIN_USE_PATTERN_JIT
    result.match = matchMethod(text, result.captures, text);
#else
    PatternContext context = {
        .start = text,
        .captures = result.captures,
    };
    result.match = root->Match(text, context);
#endif
  }
  return result;
}

PatternMatch Pattern::Search(const char *text, size_t length) const {
  PatternMatch result;
  result.match = false;
  if (length >= minimumLength) {
    result.end = text + length;
    const char *searchEnd = result.end - minimumLength;
#if JAVELIN_USE_PATTERN_JIT
    const char *start = text;
    do {
      result.match = matchMethod(start, result.captures, text);
    } while (!result.match && ++text < searchEnd);
#else
    PatternContext context = {
        .start = text,
        .captures = result.captures,
    };
    do {
      result.match = root->Match(text, context);
    } while (!result.match && ++text < searchEnd);
#endif
  }
  return result;
}

//---------------------------------------------------------------------------

char *PatternMatch::Replace(const char *format) const {
  assert(match);

  char *buffer = (char *)malloc(256);
  char *d = buffer;
  for (;;) {
    switch (*format) {
    case '\0':
      *d++ = '\0';
      assert(d <= buffer + 256);
      return (char *)realloc(buffer, d - buffer);

    case '\\': {
      ++format;
      const int index = *format - '0';
      ++format;
      assert(0 <= index && index < 4);
      const char *start = captures[index * 2];
      const char *end = captures[index * 2 + 1];
      memcpy(d, start, end - start);
      d += end - start;
    } break;

    default:
      *d++ = *format++;
      break;
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

// spellchecker: disable
TEST_BEGIN("Pattern: Simple test") {
  const Pattern pattern = Pattern::Compile("a(b|c)d");
  const char *abd = "abd";
  const PatternMatch match = pattern.Match(abd);
  assert(match.match);
  assert(match.captures[0] == abd);
  assert(match.captures[1] == abd + 3);
  assert(match.captures[2] == abd + 1);
  assert(match.captures[3] == abd + 2);

  assert(pattern.HasEndAnchor() == false);
  assert(pattern.GetMinimumLength() == 3);
  assert(pattern.GetMaximumLength() == 3);
}
TEST_END

TEST_BEGIN("Pattern: Repeat test") {
  const Pattern pattern = Pattern::Compile("a*d");
  assert(pattern.Match("d").match);
  assert(pattern.Match("ad").match);
  assert(pattern.Match("aad").match);

  assert(pattern.HasEndAnchor() == false);
  assert(pattern.GetMinimumLength() == 1);
  assert(pattern.GetMaximumLength() == size_t(-1));
}
TEST_END

TEST_BEGIN("Pattern: PlusRepeat pattern test") {
  const Pattern pattern = Pattern::Compile("a+bd");
  assert(pattern.Match("bd").match == false);
  assert(pattern.Match("abd").match);
  assert(pattern.Match("aabd").match);

  assert(pattern.HasEndAnchor() == false);
  assert(pattern.GetMinimumLength() == 3);
  assert(pattern.GetMaximumLength() == size_t(-1));
}
TEST_END

TEST_BEGIN("Pattern: Replace tests") {
  const Pattern pattern = Pattern::Compile("(a*)b");
  char *t1 = pattern.Match("aaaab").Replace("\\0\\1");
  assert(Str::Eq(t1, "aaaabaaaa"));
  free(t1);

  char *t2 = pattern.Match("b").Replace("\\0\\1");
  assert(Str::Eq(t2, "b"));
  free(t2);
}
TEST_END

TEST_BEGIN("Pattern: BackReference Test") {
  const Pattern pattern = Pattern::Compile("(.+(.))\\2ed");
  char *t1 = pattern.Match("planned").Replace("\\1");
  assert(Str::Eq(t1, "plan"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Suffix Test") {
  const Pattern pattern = Pattern::Compile("(.+)i$");
  char *t1 = pattern.Match("worthi").Replace("\\1y");
  assert(Str::Eq(t1, "worthy"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Empty alternate test") {
  const Pattern pattern = Pattern::Compile("abc|");
  assert(pattern.Match("").match);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example1 test") {
  const Pattern pattern = Pattern::Compile(
      R"(^(.*(?:[bcdfghjklmnprstvwxyz]|qu)[aeiou])([bcdfgklmnprtvz]) \^([aeiouy].*)$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 6);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("defer ^ed").Replace(R"(\1\2\2\3)");
  assert(Str::Eq(t1, "deferred"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example2 test") {
  const Pattern pattern = Pattern::Compile(R"(^(.+)e \^tiv(e|ity|ities)$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 8);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("restore ^tive").Replace(R"(\1ativ\2)");
  assert(Str::Eq(t1, "restorative"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example3 test") {
  const Pattern pattern = Pattern::Compile(R"(^(.*(?:[sz]h?|x)) \^s$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 4);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("wish ^s").Replace(R"(\1es)");
  assert(Str::Eq(t1, "wishes"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example4 test") {
  const Pattern pattern = Pattern::Compile(R"(^(.+)y \^ial(ly)?$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 7);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("industry ^ial").Replace(R"(\1ial\2)");
  assert(Str::Eq(t1, "industrial"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Orthography example5 test") {
  const Pattern pattern =
      Pattern::Compile(R"(^(.+[^aoeui])y \^if(y(?:ing)?|ie[sd]|ications?)$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 8);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("glory ^ification").Replace(R"(\1if\2)");
  assert(Str::Eq(t1, "glorification"));
  free(t1);
}
TEST_END

TEST_BEGIN("Pattern: Reverse orthography example test") {
  const Pattern pattern = Pattern::Compile(R"(^(.+(.))\2ed$)");

  assert(pattern.HasEndAnchor() == true);
  assert(pattern.GetMinimumLength() == 5);
  assert(pattern.GetMaximumLength() == size_t(-1));

  char *t1 = pattern.Match("occurred").Replace(R"(\1)");
  assert(Str::Eq(t1, "occur"));
  free(t1);
}
TEST_END
// spellchecker: enable

//---------------------------------------------------------------------------
