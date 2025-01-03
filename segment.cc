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
  const size_t limit =
      a.GetCount() < b.GetCount() ? a.GetCount() : b.GetCount();

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
  if (!endIndex) {
    return 0;
  }

  size_t index = endIndex;

  // Special case trailing spaces.
  while (index && Str::IsSpace((*this)[index].lookup.GetText())) {
    --index;
  }

  if (Str::IsFingerSpellingCommand((*this)[index].lookup.GetText())) {
    // In the case of finger spelling, keep consuming until all finger spelling
    // used.
    while (index &&
           Str::IsFingerSpellingCommand((*this)[index - 1].lookup.GetText())) {
      --index;
    }
  } else {
    while (index) {
      if (Str::HasPrefix((*this)[index].lookup.GetText(), "{^")) {
        --index;
        continue;
      }

      if (index > 0) {
        const char *previousLookup = (*this)[index - 1].lookup.GetText();
        if (Str::HasSuffix(previousLookup, "^}") &&
            !Str::IsSpace(previousLookup)) {
          --index;
          continue;
        }
      }

      break;
    }
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
      p = nullptr;
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
  const char *workingP = p;
  if (*workingP == '{') {
    while (*workingP != '}') [[likely]] {
      if (*workingP == '\0') [[unlikely]] {
        break;
      }
      if (workingP[0] == '\\') [[unlikely]] {
        if (workingP[1] != '\0') [[likely]] {
          workingP += 2;
          continue;
        }
      }
      ++workingP;
    }
    if (*workingP == '\0') [[unlikely]] {
      // Unterminated command... drop it.
      p = workingP;
      PrepareNextP();
      return StenoToken("{}", 2, state);
    }
    ++workingP;
  } else {
    for (;;) {
      switch (*workingP) {
      case '\0':
      case ' ':
      case '{':
        goto ReturnSpan;

      case '\\':
        if (workingP[1] == '\0') {
          free(scratch);
          const size_t length = workingP - start;
          scratch = Str::DupN(start, length);
          p = workingP + 1;
          PrepareNextP();
          return StenoToken(scratch, length, state);
        }
        workingP += 2;
        break;

      [[likely]] default:
        ++workingP;
      }
    }
  }

ReturnSpan:
  const char *result = start;
  const size_t length = workingP - start;
  if (*workingP != '\0') {
    free(scratch);
    result = scratch = Str::DupN(start, length);
  }

  p = workingP;
  PrepareNextP();
  return StenoToken(result, length, state);
}

void StenoSegmentListTokenizer::PrepareNextP() {
  for (;;) {
    while (*p == ' ') [[unlikely]] {
      ++p;
    }
    if (*p != '\0') [[unlikely]] {
      return;
    }
    if (elementIndex == list.GetCount()) [[unlikely]] {
      p = nullptr;
      return;
    }

    const StenoSegment &segment = list[elementIndex++];
    p = segment.lookup.GetText();
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

TEST_BEGIN("Segment tests") {
  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("-G"), StenoState());
  // spellchecker: enable

  StenoSegmentList segments;
  const StenoCompiledOrthography compiledOrthography(
      StenoOrthography::emptyOrthography);
  StenoCompactMapDictionary dictionary(TestDictionary::definition);
  StenoEngine engine(dictionary, compiledOrthography);
  BuildSegmentContext context(segments, engine, false);

  history.CreateSegments(context);

  StenoTokenizer *tokenizer = StenoTokenizer::Create(segments);

  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "test"));
  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "{^ing}"));
  assert(!tokenizer->HasMore());
  delete tokenizer;
}
TEST_END

//---------------------------------------------------------------------------
