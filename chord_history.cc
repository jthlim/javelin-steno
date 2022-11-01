//---------------------------------------------------------------------------

#include "chord_history.h"
#include "dictionary/dictionary.h"
#include "segment.h"
#include "str.h"

//---------------------------------------------------------------------------

struct AutoSuffix {
  StenoChord chord;
  const char *text;
};

constexpr AutoSuffix autoSuffixes[] = {
    {ChordMask::ZR, " {^s}"},
    {ChordMask::DR, " {^ed}"},
    {ChordMask::SR, " {^s}"},
    {ChordMask::GR, " {^ing}"},
};
const StenoChord suffixKeys(ChordMask::ZR | ChordMask::DR | ChordMask::SR |
                            ChordMask::GR);

//---------------------------------------------------------------------------

void ChordHistory::Shift() {
  if (count == 0) {
    return;
  }
  --count;
  memmove(chords, chords + 1, count * sizeof(StenoChord));
  memmove(states, states + 1, count * sizeof(StenoState));
}

//---------------------------------------------------------------------------

StenoSegmentList ChordHistory::CreateSegments(size_t maximumChordCount,
                                              const StenoDictionary &dictionary,
                                              size_t maximumChordLength,
                                              size_t minimumStartOffset) {
  size_t startOffset = maximumChordCount > maximumChordLength
                           ? maximumChordCount - maximumChordLength
                           : 0;

  if (startOffset < minimumStartOffset) {
    startOffset = minimumStartOffset;
  }

  StenoSegmentList result;
  AddSegments(result, startOffset, maximumChordCount, dictionary);
  return result;
}

void ChordHistory::AddSegments(StenoSegmentList &list, size_t offset,
                               size_t endOffset,
                               const StenoDictionary &dictionary) {
  size_t maximumMatchLength = dictionary.GetMaximumMatchLength();
  char buffer[32];

  while (offset < endOffset) {
    if (DirectLookup(list, offset, endOffset, dictionary, maximumMatchLength)) {
      continue;
    }
    if (AutoSuffixLookup(list, offset, endOffset, dictionary,
                         maximumMatchLength)) {
      continue;
    }

    chords[offset].ToString(buffer);
    list.Add(StenoSegment(
        1, states + offset,
        StenoDictionaryLookup::CreateDynamicString(Str::Dup(buffer))));
    ++offset;
  }
}

bool ChordHistory::DirectLookup(StenoSegmentList &list, size_t &offset,
                                size_t endOffset,
                                const StenoDictionary &dictionary,
                                size_t maximumMatchLength) {
  size_t startLength = endOffset - offset;
  if (startLength > maximumMatchLength) {
    startLength = maximumMatchLength;
  }

  for (size_t length = startLength; length > 0; --length) {
    StenoDictionaryLookup lookup = dictionary.Lookup(chords + offset, length);
    if (lookup.IsValid()) {
      if (strstr(lookup.GetText(), "{*?}")) {
        HandleRetroactiveInsertSpace(list, dictionary);
        lookup.Destroy();
      } else {
        list.Add(StenoSegment(length, states + offset, lookup));
      }

      offset += length;
      return true;
    }
  }

  return false;
}

bool ChordHistory::AutoSuffixLookup(StenoSegmentList &list, size_t &offset,
                                    size_t endOffset,
                                    const StenoDictionary &dictionary,
                                    size_t maximumMatchLength) {

  // See which historical translations can be extended with auto-suffixes.
  const StenoState *oldestState =
      offset < maximumMatchLength ? states
                                  : states + offset + 1 - maximumMatchLength;
  for (size_t segmentIndex = 0; segmentIndex < list.GetCount();
       ++segmentIndex) {
    if (list[segmentIndex].state < oldestState) {
      continue;
    }

    StenoSegment segment = AutoSuffixTest(list[segmentIndex], offset,
                                          dictionary, maximumMatchLength);

    if (segment.IsValid()) {
      // There's a match! Pop off history.
      while (list.GetCount() > segmentIndex + 1) {
        list.Back().lookup.Destroy();
        list.Pop();
      }
      list.Back().lookup.Destroy();
      list.Back() = segment;
      offset = segment.state - states + segment.chordLength;
      return true;
    }
  }

  // No historical translations matched. Try auto suffixing at this offset.
  StenoSegment segment = AutoSuffixTest(offset,
                                        endOffset - offset > maximumMatchLength
                                            ? maximumMatchLength
                                            : endOffset - offset,
                                        1, dictionary);

  if (!segment.IsValid()) {
    return false;
  }

  // Auto-suffix worked. Add it to the list.
  list.Add(segment);
  offset += segment.chordLength;
  return true;
}

StenoSegment ChordHistory::AutoSuffixTest(const StenoSegment &segment,
                                          size_t offset,
                                          const StenoDictionary &dictionary,
                                          size_t maximumMatchLength) {
  size_t lastChordOffset = segment.state - states;
  size_t startLength = count - lastChordOffset > maximumMatchLength
                           ? maximumMatchLength
                           : count - lastChordOffset;
  size_t minimumLength = offset - lastChordOffset + 1;
  return AutoSuffixTest(lastChordOffset, startLength, minimumLength,
                        dictionary);
}

StenoSegment ChordHistory::AutoSuffixTest(size_t offset, size_t startLength,
                                          size_t minimumLength,
                                          const StenoDictionary &dictionary) {
  StenoChord *localChords =
      (StenoChord *)alloca(sizeof(StenoChord) * startLength);
  memcpy(localChords, chords + offset, sizeof(StenoChord) * startLength);

  for (size_t length = startLength; length >= minimumLength; --length) {
    if ((chords[offset + length - 1] & suffixKeys).IsEmpty()) {
      continue;
    }

    for (const AutoSuffix &suffix : autoSuffixes) {
      if ((chords[offset + length - 1] & suffix.chord).IsEmpty()) {
        continue;
      }
      localChords[length - 1] = chords[offset + length - 1] & ~suffix.chord;

      StenoDictionaryLookup lookup = dictionary.Lookup(localChords, length);
      if (lookup.IsValid()) {
        const char *text = lookup.GetText();
        const char *result = Str::Asprintf("%s%s", text, suffix.text);
        lookup.Destroy();
        return StenoSegment(length, states + offset,
                            StenoDictionaryLookup::CreateDynamicString(result));
      }
    }
  }

  return StenoSegment(0, nullptr, StenoDictionaryLookup::CreateInvalid());
}

void ChordHistory::HandleRetroactiveInsertSpace(
    StenoSegmentList &list, const StenoDictionary &dictionary) {
  // The last element needs to be split up.
  if (list.IsEmpty()) {
    return;
  }

  StenoSegment &lastSegment = list.Back();
  if (!lastSegment.IsValid()) {
    return;
  }
  size_t chordLength = lastSegment.chordLength;
  lastSegment.lookup.Destroy();

  size_t offset = lastSegment.state - states;

  list.Pop();

  AddSegments(list, offset, offset + chordLength - 1, dictionary);
  AddSegments(list, offset + chordLength - 1, offset + chordLength, dictionary);
}

//---------------------------------------------------------------------------

#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "str.h"
#include "unit_test.h"
#include <stdio.h>

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

const StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

constexpr StenoDictionaryList dictionary(DICTIONARIES, 2);

TEST_BEGIN("ChordHistory: Test single segment") {
  ChordHistory history;
  history.Add(StenoChord("TEFT"), StenoState());

  StenoSegmentList segmentList =
      history.CreateSegments(history.GetCount(), dictionary, 32);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
}
TEST_END

TEST_BEGIN("ChordHistory: Test two segments, with multi-stroke") {
  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("-D"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList =
      history.CreateSegments(history.GetCount(), dictionary, 32);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "tested"));
}
TEST_END

TEST_BEGIN("ChordHistory: Test *? splits strokes") {
  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("-D"), StenoState());
  history.Add(StenoChord("SKWHU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList =
      history.CreateSegments(history.GetCount(), dictionary, 32);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "-D"));
}
TEST_END

//---------------------------------------------------------------------------
