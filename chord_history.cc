//---------------------------------------------------------------------------

#include "chord_history.h"
#include "dictionary/dictionary.h"
#include "orthography.h"
#include "segment.h"
#include "str.h"

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

void ChordHistory::CreateSegments(BuildSegmentContext &context,
                                  size_t maximumChordLength,
                                  size_t minimumStartOffset) {
  size_t startOffset = context.endOffset > maximumChordLength
                           ? context.endOffset - maximumChordLength
                           : 0;

  if (startOffset < minimumStartOffset) {
    startOffset = minimumStartOffset;
  }

  AddSegments(context, startOffset);
}

void ChordHistory::AddSegments(BuildSegmentContext &context, size_t offset) {
  char buffer[32];

  while (offset < context.endOffset) {
    if (DirectLookup(context, offset)) {
      continue;
    }
    if (AutoSuffixLookup(context, offset)) {
      continue;
    }

    chords[offset].ToString(buffer);
    context.segmentList.Add(StenoSegment(
        1, states + offset,
        StenoDictionaryLookup::CreateDynamicString(Str::Dup(buffer))));
    ++offset;
  }
}

bool ChordHistory::DirectLookup(BuildSegmentContext &context, size_t &offset) {
  size_t startLength = context.endOffset - offset;
  if (startLength > context.maximumMatchLength) {
    startLength = context.maximumMatchLength;
  }

  for (size_t length = startLength; length > 0; --length) {
    StenoDictionaryLookup lookup =
        context.dictionary.Lookup(chords + offset, length);
    if (lookup.IsValid()) {
      if (strstr(lookup.GetText(), "{*?}")) {
        HandleRetroactiveInsertSpace(context, offset);
        lookup.Destroy();
      } else {
        context.segmentList.Add(StenoSegment(length, states + offset, lookup));
      }

      offset += length;
      return true;
    }
  }

  return false;
}

bool ChordHistory::AutoSuffixLookup(BuildSegmentContext &context,
                                    size_t &offset) {

  // See which historical translations can be extended with auto-suffixes.
  const StenoState *oldestState =
      offset < context.maximumMatchLength
          ? states
          : states + offset + 1 - context.maximumMatchLength;
  for (size_t segmentIndex = 0; segmentIndex < context.segmentList.GetCount();
       ++segmentIndex) {
    if (context.segmentList[segmentIndex].state < oldestState) {
      continue;
    }

    StenoSegment segment =
        AutoSuffixTest(context, context.segmentList[segmentIndex], offset);

    if (segment.IsValid()) {
      // There's a match! Pop off history.
      while (context.segmentList.GetCount() > segmentIndex + 1) {
        context.segmentList.Back().lookup.Destroy();
        context.segmentList.Pop();
      }
      context.segmentList.Back().lookup.Destroy();
      context.segmentList.Back() = segment;
      offset = segment.state - states + segment.chordLength;
      return true;
    }
  }

  // No historical translations matched. Try auto suffixing at this offset.
  StenoSegment segment =
      AutoSuffixTest(context, offset,
                     context.endOffset - offset > context.maximumMatchLength
                         ? context.maximumMatchLength
                         : context.endOffset - offset,
                     1);

  if (!segment.IsValid()) {
    return false;
  }

  // Auto-suffix worked. Add it to the list.
  context.segmentList.Add(segment);
  offset += segment.chordLength;
  return true;
}

StenoSegment ChordHistory::AutoSuffixTest(BuildSegmentContext &context,
                                          const StenoSegment &segment,
                                          size_t offset) {
  size_t lastChordOffset = segment.state - states;
  size_t startLength =
      context.endOffset - lastChordOffset > context.maximumMatchLength
          ? context.maximumMatchLength
          : context.endOffset - lastChordOffset;
  size_t minimumLength = offset - lastChordOffset + 1;
  return AutoSuffixTest(context, lastChordOffset, startLength, minimumLength);
}

StenoSegment ChordHistory::AutoSuffixTest(BuildSegmentContext &context,
                                          size_t offset, size_t startLength,
                                          size_t minimumLength) {
  StenoChord *localChords =
      (StenoChord *)alloca(sizeof(StenoChord) * startLength);
  memcpy(localChords, chords + offset, sizeof(StenoChord) * startLength);

  const StenoOrthography &orthography = context.orthography.orthography;

  for (size_t length = startLength; length >= minimumLength; --length) {
    if ((chords[offset + length - 1] & orthography.autoSuffixMask).IsEmpty()) {
      continue;
    }

    for (size_t i = 0; i < orthography.autoSuffixCount; ++i) {
      const StenoOrthographyAutoSuffix &suffix = orthography.autoSuffixes[i];
      if ((chords[offset + length - 1] & suffix.chord).IsEmpty()) {
        continue;
      }
      localChords[length - 1] = chords[offset + length - 1] & ~suffix.chord;

      StenoDictionaryLookup lookup =
          context.dictionary.Lookup(localChords, length);
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

void ChordHistory::HandleRetroactiveInsertSpace(BuildSegmentContext &context,
                                                size_t currentOffset) {
  // The last element needs to be split up.
  if (context.segmentList.IsEmpty()) {
    return;
  }

  StenoSegment &lastSegment = context.segmentList.Back();
  if (!lastSegment.IsValid()) {
    return;
  }
  size_t chordLength = lastSegment.chordLength;

  if (chordLength == 1) {
    // Put in an extra empty segment to prevent this chord from being recorded.
    // (See logic at end of StenoEngine::ProcessNormalModeChord).
    context.segmentList.Add(
        StenoSegment(1, states + currentOffset,
                     StenoDictionaryLookup::CreateStaticString("")));
    return;
  }

  lastSegment.lookup.Destroy();

  size_t offset = lastSegment.state - states;

  context.segmentList.Pop();

  BuildSegmentContext firstSegment(
      context.segmentList, context.dictionary, context.maximumMatchLength,
      context.orthography, offset + chordLength - 1);

  AddSegments(firstSegment, offset);

  BuildSegmentContext secondSegment(context.segmentList, context.dictionary, 1,
                                    context.orthography, offset + chordLength);

  AddSegments(secondSegment, offset + chordLength - 1);
}

//---------------------------------------------------------------------------

#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "orthography.h"
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
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              history.GetCount());
  history.CreateSegments(context, 32);

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

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              history.GetCount());
  history.CreateSegments(context, 32);

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

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              history.GetCount());
  history.CreateSegments(context, 32);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "-D"));
}
TEST_END

//---------------------------------------------------------------------------
