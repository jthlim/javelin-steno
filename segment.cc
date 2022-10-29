//---------------------------------------------------------------------------

#include "segment.h"
#include "string_util.h"
#include <stdio.h>

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
  virtual ~StenoSegmentListTokenizer() { free(scratch); }

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
          scratch = strndup(start, p - start);
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
    result = scratch = strndup(start, p - start);
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

#include "chord_history.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "string_util.h"
#include "unit_test.h"
#include <stdio.h>

constexpr StenoMapDictionary dictionary(MainDictionary::definition);

TEST_BEGIN("Segment tests") {
  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("-G"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList = history.CreateSegments(dictionary, 10);
  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();

  assert(tokenizer->HasMore());
  assert(streq(tokenizer->GetNext().text, "test"));
  assert(tokenizer->HasMore());
  assert(streq(tokenizer->GetNext().text, "{^ing}"));
  assert(!tokenizer->HasMore());
  delete tokenizer;
}
TEST_END

//---------------------------------------------------------------------------
