//---------------------------------------------------------------------------

#include "segment.h"
#include "str.h"

//---------------------------------------------------------------------------

bool StenoSegment::ContainsKeyCode() const {
  return Str::ContainsKeyCode(lookup.GetText());
}

bool StenoSegment::HasCommand() const { return strchr(lookup.GetText(), '{'); }

//---------------------------------------------------------------------------

StenoSegmentList::~StenoSegmentList() {
  for (size_t i = 0; i < count; ++i) {
    (*this)[i].lookup.Destroy();
  }
}

size_t
StenoSegmentList::GetCommonStartingSegmentsCount(const List<StenoSegment> &a,
                                                 const List<StenoSegment> &b) {
  size_t limit = a.GetCount() < b.GetCount() ? a.GetCount() : b.GetCount();

  size_t commonPrefixCount = 0;
  for (; commonPrefixCount < limit; ++commonPrefixCount) {
    if (a[commonPrefixCount].lookup == b[commonPrefixCount].lookup) {
      continue;
    }
    if (!Str::Eq(a[commonPrefixCount].lookup.GetText(),
                 b[commonPrefixCount].lookup.GetText())) {
      break;
    }
  }

  // For suffixes to work, check if the next segment has a command in it.
  if (commonPrefixCount < a.GetCount()) {
    if (a[commonPrefixCount].HasCommand()) {
      return 0;
    }
  }
  if (commonPrefixCount < b.GetCount()) {
    if (b[commonPrefixCount].HasCommand()) {
      return 0;
    }
  }

  return commonPrefixCount;
}

size_t StenoSegmentList::GetWordStartingSegmentIndex(size_t endIndex) const {
  if (!endIndex)
    return 0;

  size_t index = endIndex;
  if (Str::IsFingerSpellingCommand((*this)[index].lookup.GetText())) {
    // In the case of finger spelling, keep consuming until all finger spelling
    // used.
    while (index &&
           Str::IsFingerSpellingCommand((*this)[index - 1].lookup.GetText())) {
      --index;
    }
    return index;
  }

  while (index) {
    if (Str::HasPrefix((*this)[index].lookup.GetText(), "{^")) {
      --index;
      continue;
    }

    if (index > 0) {
      const char *previousLookup = (*this)[index - 1].lookup.GetText();
      if (Str::HasSuffix(previousLookup, "^}") &&
          !Str::Eq(previousLookup, "{^ ^}")) {
        --index;
        continue;
      }
    }

    break;
  }
  return index;
}

//---------------------------------------------------------------------------

class StenoSegmentListTokenizer final : public StenoTokenizer {
public:
  StenoSegmentListTokenizer(const List<StenoSegment> &list,
                            size_t startingOffset)
      : list(list), elementIndex(startingOffset) {
    if (list.IsEmpty()) {
      p = elementText = nullptr;
    } else {
      p = "";
      PrepareNextP();
    }
  }
  virtual ~StenoSegmentListTokenizer() { free(scratch); }

  bool HasMore() const final { return p != nullptr; }

  StenoToken GetNext() final;

private:
  const List<StenoSegment> &list;
  size_t elementIndex;
  const char *elementText;
  const char *p;
  const StenoState *nextState = nullptr;

  char *scratch = nullptr;

  void PrepareNextP();
};

StenoToken StenoSegmentListTokenizer::GetNext() {
  const StenoState *state = nextState;
  nextState = nullptr;

  assert(p != nullptr);
  const char *start = p;
  if (*p == '{') {
    while (*p != '}' && *p != '\0') {
      if (p[0] == '\\' && p[1] != '\0') {
        ++p;
      }
      ++p;
    }
    if (*p == '\0') {
      // Unterminated command... drop it.
      PrepareNextP();
      return StenoToken("{}", 2, state);
    }
    ++p;
  } else {
    for (;;) {
      switch (*p) {
      case '\0':
      case ' ':
      case '{':
        goto ReturnSpan;

      case '\\':
        if (p[1] == '\0') {
          free(scratch);
          size_t length = p - start;
          scratch = Str::DupN(start, length);
          ++p;
          PrepareNextP();
          return StenoToken(scratch, length, state);
        }
        p += 2;
        break;

      default:
        ++p;
      }
    }
  }

ReturnSpan:
  const char *result = elementText;
  size_t length = p - start;
  if (start != elementText || *p != '\0') {
    free(scratch);
    result = scratch = Str::DupN(start, length);
  }

  PrepareNextP();
  return StenoToken(result, length, state);
}

void StenoSegmentListTokenizer::PrepareNextP() {
  for (;;) {
    while (*p == ' ') {
      ++p;
    }
    if (*p != '\0') {
      return;
    }
    if (elementIndex == list.GetCount()) {
      p = elementText = nullptr;
      return;
    }

    const StenoSegment &segment = list[elementIndex++];
    p = elementText = segment.lookup.GetText();
    nextState = segment.state;
  }
}

StenoTokenizer *StenoTokenizer::Create(const List<StenoSegment> &segments,
                                       size_t startingOffset) {
  return new StenoSegmentListTokenizer(segments, startingOffset);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "dictionary/compact_map_dictionary.h"
#include "dictionary/test_dictionary.h"
#include "engine.h"
#include "orthography.h"
#include "segment_builder.h"
#include "str.h"
#include "unit_test.h"

static StenoCompactMapDictionary dictionary(TestDictionary::definition);

TEST_BEGIN("Segment tests") {
  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("-G"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  StenoCompiledOrthography compiledOrthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(dictionary, compiledOrthography);
  BuildSegmentContext context(segmentList, engine);

  history.CreateSegments(context);

  StenoTokenizer *tokenizer = StenoTokenizer::Create(segmentList);

  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "test"));
  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "{^ing}"));
  assert(!tokenizer->HasMore());
  delete tokenizer;
}
TEST_END

//---------------------------------------------------------------------------
