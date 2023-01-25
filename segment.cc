//---------------------------------------------------------------------------

#include "segment.h"
#include "str.h"

//---------------------------------------------------------------------------

bool StenoSegment::ContainsKeyCode() const {
  return Str::ContainsKeyCode(lookup.GetText());
}

//---------------------------------------------------------------------------

StenoSegmentList::~StenoSegmentList() {
  for (size_t i = 0; i < count; ++i) {
    (*this)[i].lookup.Destroy();
  }
}

//---------------------------------------------------------------------------

class StenoSegmentListTokenizer final : public StenoTokenizer {
public:
  StenoSegmentListTokenizer(const StenoSegmentList &list) : list(list) {
    if (list.IsEmpty()) {
      p = elementText = nullptr;
    } else {
      p = "";
      PrepareNextP();
    }
  }
  virtual ~StenoSegmentListTokenizer() {
    if (scratch) {
      free(scratch);
    }
  }

  bool HasMore() const final { return p != nullptr; }

  StenoToken GetNext() final;

private:
  const StenoSegmentList &list;
  size_t elementIndex = 0;
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
      ++p;
    }
    if (*p == '\0') {
      // Unterminated command... drop it.
      PrepareNextP();
      return StenoToken("{}", state);
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
          scratch = Str::DupN(start, p - start);
          ++p;
          PrepareNextP();
          return StenoToken(scratch, state);
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
  if (start != elementText || *p != '\0') {
    free(scratch);
    result = scratch = Str::DupN(start, p - start);
  }

  PrepareNextP();
  return StenoToken(result, state);
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

StenoTokenizer *StenoSegmentList::CreateTokenizer() {
  return new StenoSegmentListTokenizer(*this);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "orthography.h"
#include "str.h"
#include "stroke_history.h"
#include "unit_test.h"

constexpr StenoMapDictionary dictionary(MainDictionary::definition);

TEST_BEGIN("Segment tests") {
  StenoStrokeHistory history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("-G"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  StenoCompiledOrthography compiledOrthography(
      StenoOrthography::emptyOrthography);
  BuildSegmentContext context(segmentList, dictionary, compiledOrthography);

  history.CreateSegments(context);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();

  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "test"));
  assert(tokenizer->HasMore());
  assert(Str::Eq(tokenizer->GetNext().text, "{^ing}"));
  assert(!tokenizer->HasMore());
  delete tokenizer;
}
TEST_END

//---------------------------------------------------------------------------
