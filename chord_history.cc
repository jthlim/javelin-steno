//---------------------------------------------------------------------------

#include "chord_history.h"
#include "dictionary/dictionary.h"
#include "orthography.h"
#include "segment.h"
#include "str.h"

//---------------------------------------------------------------------------

BuildSegmentContext::BuildSegmentContext(
    StenoSegmentList &segmentList, const StenoDictionary &dictionary,
    const StenoCompiledOrthography &orthography)
    : segmentList(segmentList), dictionary(dictionary),
      maximumMatchLength(dictionary.GetMaximumMatchLength()),
      orthography(orthography) {}

//---------------------------------------------------------------------------

void ChordHistory::Shift() {
  if (count == 0) {
    return;
  }
  --count;
  memmove(chords, chords + 1, count * sizeof(StenoChord));
  memmove(states, states + 1, count * sizeof(StenoState));
}

size_t ChordHistory::GetUndoCount(size_t maxCount) const {
  if (count == 0) {
    return 0;
  }

  size_t result = 1;
  while (result < maxCount && result < count &&
         states[count - result].shouldCombineUndo) {
    ++result;
  }
  return result;
}

void ChordHistory::TransferFrom(const ChordHistory &source,
                                size_t sourceChordCount, size_t maxCount) {
  size_t offset =
      sourceChordCount <= maxCount ? 0 : sourceChordCount - maxCount;
  count = sourceChordCount - offset;

  memcpy(chords, source.chords + offset, count * sizeof(StenoChord));
  memcpy(states, source.states + offset, count * sizeof(StenoState));
}

void ChordHistory::CreateSegments(BuildSegmentContext &context,
                                  size_t minimumStartOffset) {
  AddSegments(context, minimumStartOffset);
}

void ChordHistory::AddSegments(BuildSegmentContext &context, size_t offset) {
  char buffer[32];

  while (offset < count) {
    if (DirectLookup(context, offset)) {
      continue;
    }
    if (AutoSuffixLookup(context, offset)) {
      continue;
    }

    chords[offset].ToString(buffer);
    context.segmentList.Add(StenoSegment(
        1, states + offset,
        StenoDictionaryLookupResult::CreateDynamicString(Str::Dup(buffer))));
    ++offset;
  }
}

bool ChordHistory::DirectLookup(BuildSegmentContext &context, size_t &offset) {
  size_t startLength = count - offset;
  if (startLength > context.maximumMatchLength) {
    startLength = context.maximumMatchLength;
  }

  for (size_t length = startLength; length > 0; --length) {
    StenoDictionaryLookupResult lookup =
        context.dictionary.Lookup(chords + offset, length);

    if (lookup.IsValid()) {
      const char *lookupText = lookup.GetText();
      if (strstr(lookupText, "{*")) {
        if (strstr(lookupText, "{*?}")) {
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRetroactiveInsertSpace(context, offset);
          ReevaluateSegments(context, offset);
          return true;
        }
        if (strstr(lookupText, "{*}")) {
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRetroactiveToggleAsterisk(context, offset);
          ReevaluateSegments(context, offset);
          return true;
        }
        if (strstr(lookupText, "{*+}")) {
          StenoState state = states[offset];
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRepeatLastStroke(context, offset, state);
          ReevaluateSegments(context, offset);
          return true;
        }
      }

      context.segmentList.Add(StenoSegment(length, states + offset, lookup));
      offset += length;
      return true;
    }
  }

  return false;
}

void ChordHistory::RemoveOffset(BuildSegmentContext &context, size_t &offset,
                                size_t length) {
  assert(offset + length <= count);
  size_t remaining = count - offset - length;
  memmove(chords + offset, chords + offset + length,
          sizeof(StenoChord) * remaining);
  memmove(states + offset, states + offset + length,
          sizeof(StenoState) * remaining);
  count -= length;
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
  StenoSegment segment = AutoSuffixTest(
      context, offset,
      count - offset > context.maximumMatchLength ? context.maximumMatchLength
                                                  : count - offset,
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
  size_t startLength = count - lastChordOffset > context.maximumMatchLength
                           ? context.maximumMatchLength
                           : count - lastChordOffset;
  size_t minimumLength = offset - lastChordOffset + 1;
  return AutoSuffixTest(context, lastChordOffset, startLength, minimumLength);
}

StenoSegment ChordHistory::AutoSuffixTest(BuildSegmentContext &context,
                                          size_t offset, size_t startLength,
                                          size_t minimumLength) {
  StenoChord *localChords =
      (StenoChord *)alloca(sizeof(StenoChord) * startLength);
  memcpy(localChords, chords + offset, sizeof(StenoChord) * startLength);

  const StenoOrthography &orthography = context.orthography.data;

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

      StenoDictionaryLookupResult lookup =
          context.dictionary.Lookup(localChords, length);

      if (lookup.IsValid()) {
        const char *text = lookup.GetText();
        const char *result = Str::Asprintf("%s%s", text, suffix.text);
        lookup.Destroy();
        return StenoSegment(
            length, states + offset,
            StenoDictionaryLookupResult::CreateDynamicString(result));
      }
    }
  }

  return StenoSegment(0, nullptr, StenoDictionaryLookupResult::CreateInvalid());
}

void ChordHistory::ReevaluateSegments(BuildSegmentContext &context,
                                      size_t &offset) {
  size_t currentOffset = offset;
  while (context.segmentList.IsNotEmpty()) {
    StenoSegment &lastSegment = context.segmentList.Back();
    size_t lastOffset = lastSegment.state - states;
    if (lastOffset + context.maximumMatchLength < currentOffset) {
      return;
    }
    lastSegment.lookup.Destroy();
    context.segmentList.Pop();
    offset = lastOffset;
  }
}

void ChordHistory::HandleRetroactiveInsertSpace(BuildSegmentContext &context,
                                                size_t currentOffset) {
  if (currentOffset == 0) {
    return;
  }

  ++count;
  size_t remaining = count - currentOffset;
  memmove(chords + currentOffset, chords + currentOffset - 1,
          sizeof(StenoChord) * remaining);
  memmove(states + currentOffset, states + currentOffset - 1,
          sizeof(StenoState) * remaining);

  chords[currentOffset - 1] = StenoChord(0);
}

void ChordHistory::HandleRetroactiveToggleAsterisk(BuildSegmentContext &context,
                                                   size_t currentOffset) {
  if (currentOffset == 0) {
    return;
  }

  chords[currentOffset - 1] ^= ChordMask::STAR;
}

void ChordHistory::HandleRepeatLastStroke(BuildSegmentContext &context,
                                          size_t currentOffset,
                                          const StenoState &state) {
  if (currentOffset == 0) {
    return;
  }

  size_t remaining = count - currentOffset;
  memmove(chords + currentOffset + 1, chords + currentOffset,
          sizeof(StenoChord) * remaining);
  memmove(states + currentOffset + 1, states + currentOffset,
          sizeof(StenoState) * remaining);

  chords[currentOffset] = chords[currentOffset - 1];
  states[currentOffset] = state;
  ++count;
}

//---------------------------------------------------------------------------

#include "dictionary/debug_dictionary.h"
#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "orthography.h"
#include "str.h"
#include "unit_test.h"

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

const StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

TEST_BEGIN("ChordHistory: Test single segment") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
}
TEST_END

TEST_BEGIN("ChordHistory: Test two segments, with multi-stroke") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("-D"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "tested"));
}
TEST_END

TEST_BEGIN("ChordHistory: Test *? splits strokes") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("-D"), StenoState());
  history.Add(StenoChord("SKWHU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 3);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), ""));
  assert(Str::Eq(segmentList[2].lookup.GetText(), "-D"));
}
TEST_END

TEST_BEGIN("ChordHistory: Test * toggles ") {
  StenoDebugDictionary dictionary;
  dictionary.SetResponse("{*}");

  ChordHistory history;
  // spellchecker: disable
  history.Add(StenoChord("TEFT"), StenoState());
  history.Add(StenoChord("#EU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "T*EFT"));
}
TEST_END

//---------------------------------------------------------------------------
