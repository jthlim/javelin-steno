
//---------------------------------------------------------------------------

#include "segment_builder.h"
#include "dictionary/dictionary.h"
#include "engine.h"
#include "orthography.h"
#include "segment.h"
#include "state.h"
#include "str.h"
#include "stroke_history.h"
#include "unicode.h"
#include "utf8_pointer.h"
#include "writer.h"

//---------------------------------------------------------------------------

BuildSegmentContext::BuildSegmentContext(StenoSegmentList &segmentList,
                                         StenoEngine &engine,
                                         bool allowSetValue)
    : allowSetValue(allowSetValue), segmentList(segmentList), engine(engine),
      dictionary(engine.GetDictionary()), orthography(engine.GetOrthography()),
      maximumOutlineLength(dictionary.GetMaximumOutlineLength()) {}

//---------------------------------------------------------------------------

void StenoSegmentBuilder::TransferStartFrom(const StenoSegmentBuilder &source,
                                            size_t count) {
  source.strokes->CopyTo(strokes, count);
  source.states->CopyTo(states, count);
  hasModifiedStrokeHistory = false;
}

void StenoSegmentBuilder::TransferFrom(const StenoStrokeHistory &source,
                                       size_t sourceStrokeCount,
                                       size_t maxCount) {
  size_t offset =
      sourceStrokeCount <= maxCount ? 0 : sourceStrokeCount - maxCount;
  count = sourceStrokeCount - offset;

  for (size_t i = 0; i < count; ++i) {
    const StenoStrokeHistoryEntry &entry = source[offset + i];
    strokes[i] = entry.stroke;
    states[i] = entry.state;
  }
  hasModifiedStrokeHistory = false;
}

void StenoSegmentBuilder::AddSegments(BuildSegmentContext &context,
                                      size_t &offset) {
  while (offset < count) {
    if (DirectLookup(context, offset)) {
      continue;
    }
    if (AutoSuffixLookup(context, offset)) {
      continue;
    }

    AddRawStroke(context, offset);
  }
}

size_t
StenoSegmentBuilder::GetFirstDefinitionBoundaryLength(size_t offset,
                                                      size_t length) const {
  for (size_t i = 1; i < length; ++i) {
    if (states[offset + i].IsDefinitionStart()) {
      return i;
    }
  }
  return 0;
}

size_t StenoSegmentBuilder::GetStartingDefinitionLength(
    size_t offset, size_t maximumOutlineLength) const {
  for (size_t i = 1; i < maximumOutlineLength; ++i) {
    if (states[offset + i].IsDefinitionStart()) {
      return i;
    }
  }
  return maximumOutlineLength;
}

bool StenoSegmentBuilder::DirectLookup(BuildSegmentContext &context,
                                       size_t &offset) {
  size_t startLength = count - offset;
  if (startLength > context.maximumOutlineLength) {
    startLength =
        GetStartingDefinitionLength(offset, context.maximumOutlineLength);
  }

  size_t length = startLength;
  while (length > 0) {
    StenoDictionaryLookupResult lookup =
        context.dictionary.Lookup(strokes + offset, length);

    if (!lookup.IsValid()) {
      if (hasModifiedStrokeHistory ||
          !states[offset + length - 1].IsDefinitionStart()) {
        --length;
      } else if (states[offset].lookupType != SegmentLookupType::DIRECT) {
        return false;
      } else {
        length = GetFirstDefinitionBoundaryLength(offset, length);
      }
      continue;
    }

    const char *lookupText = lookup.GetText();

    if (lookupText[0] == '=') {
      if (Str::HasPrefix(lookupText, "=retro_transform:")) {
        if (context.segmentList.IsEmpty()) {
          context.segmentList.Add(
              StenoSegment(length, SegmentLookupType::DIRECT, states + offset,
                           StenoDictionaryLookupResult::CreateDynamicString(
                               EscapeCommand(lookupText))));
          offset += length;
        } else {
          HandleRetroTransform(context, lookupText, offset, length);
          offset += length;
        }

        lookup.Destroy();
        return true;
      }
      if (Str::HasPrefix(lookupText, "=set_value:")) {
        if (context.segmentList.IsEmpty()) {
          context.segmentList.Add(
              StenoSegment(length, SegmentLookupType::DIRECT, states + offset,
                           StenoDictionaryLookupResult::CreateDynamicString(
                               EscapeCommand(lookupText))));
          offset += length;
        } else {
          HandleRetroSetValue(context, lookupText, offset, length);
          offset += length;
        }

        lookup.Destroy();
        return true;
      }
      if (Str::HasPrefix(lookupText, "=transform:")) {
        const char *format = lookupText + sizeof("=transform:") - 1;
        BufferWriter writer;
        EscapeCommand(writer, lookupText);
        CreateTransformString(writer, context, format);
        char *result = writer.TerminateStringAndAdoptBuffer();
        context.segmentList.Add(StenoSegment(
            length, SegmentLookupType::DIRECT, states + offset,
            StenoDictionaryLookupResult::CreateDynamicString(result)));
        offset += length;

        lookup.Destroy();
        return true;
      }
      if (Str::Eq(lookupText, "=retro_toggle_asterisk")) {
        if (offset + length == count) {
          context.lastSegmentCommand = "=retro_toggle_asterisk";
        }
        goto HandleRetroToggleAsterisk;
      }
      if (Str::Eq(lookupText, "=retro_insert_space")) {
        if (offset + length == count) {
          context.lastSegmentCommand = "=retro_insert_space";
        }
        goto HandleRetroInsertSpace;
      }
      if (Str::Eq(lookupText, "=repeat_last_stroke")) {
        if (offset + length == count) {
          context.lastSegmentCommand = "=repeat_last_stroke";
        }
        goto HandleRepeatLastStroke;
      }
    }

    if (lookupText[0] == '{' && lookupText[1] == '*') {
      if (lookupText[2] == '?' && lookupText[3] == '}') { // {*?}
        if (offset + length == count) {
          context.lastSegmentCommand = "{*?}";
        }
      HandleRetroInsertSpace:
        lookup.Destroy();
        HandleRetroInsertSpace(context, offset, length);
        ReevaluateSegments(context, offset);
        return true;
      } else if (lookupText[2] == '}') { // {*}
        if (offset + length == count) {
          context.lastSegmentCommand = "{*}";
        }
      HandleRetroToggleAsterisk:
        lookup.Destroy();
        HandleRetroToggleAsterisk(context, offset, length);
        ReevaluateSegments(context, offset);
        return true;
      } else if (lookupText[2] == '+' && lookupText[3] == '}') { // {*+}
        if (offset + length == count) {
          context.lastSegmentCommand = "{*+}";
        }
      HandleRepeatLastStroke:
        lookup.Destroy();
        HandleRepeatLastStroke(context, offset, length);
        ReevaluateSegments(context, offset);
        return true;
      }
    }

    context.segmentList.Add(StenoSegment(length, SegmentLookupType::DIRECT,
                                         states + offset, lookup));
    offset += length;
    return true;
  }

  return false;
}

bool StenoSegmentBuilder::AutoSuffixLookup(BuildSegmentContext &context,
                                           size_t &offset) {

  // See which historical translations can be extended with auto-suffixes.
  const StenoState *oldestState =
      offset < context.maximumOutlineLength
          ? states
          : states + offset + 1 - context.maximumOutlineLength;

  size_t startingSegment = context.segmentList.GetCount();
  while (startingSegment > 0) {
    if (context.segmentList[startingSegment - 1].state < oldestState) {
      break;
    }

    // If a previous segment is not direct, then it would have already attempted
    // to autosuffix as long as possible, so doesn't need to be extended.
    if (context.segmentList[startingSegment - 1].lookupType !=
        SegmentLookupType::DIRECT) {
      break;
    }
    --startingSegment;
  }

  for (StenoSegment &existingSegment :
       context.segmentList.Skip(startingSegment)) {
    StenoSegment segment = AutoSuffixTest(context, existingSegment, offset);
    if (!segment.IsValid()) {
      continue;
    }

    // There's a match! Pop off history.
    offset = segment.GetEndStrokeIndex(states);
    StenoSegmentList &segmentList = context.segmentList;
    for (;;) {
      StenoSegment &back = segmentList.Back();
      back.lookup.Destroy();
      if (&back != &existingSegment) {
        segmentList.Pop();

        continue;
      }
      existingSegment = segment;
      return true;
    }
  }

  // No historical translations matched. Try auto suffixing at this offset.
  size_t startLength = count - offset;
  if (startLength > context.maximumOutlineLength) {
    if (!hasModifiedStrokeHistory &&
        states[offset].lookupType != SegmentLookupType::AUTO_SUFFIX) {
      return false;
    }
    startLength =
        GetStartingDefinitionLength(offset, context.maximumOutlineLength);
  }
  StenoSegment segment = AutoSuffixTest(context, offset, startLength, 1);

  if (!segment.IsValid()) {
    return false;
  }

  // Auto-suffix worked. Add it to the list.
  context.segmentList.Add(segment);
  offset += segment.strokeLength;
  return true;
}

StenoSegment StenoSegmentBuilder::AutoSuffixTest(BuildSegmentContext &context,
                                                 const StenoSegment &segment,
                                                 size_t offset) {
  size_t lastStrokeOffset = segment.GetStrokeIndex(states);
  size_t startLength = count - lastStrokeOffset;
  if (startLength > context.maximumOutlineLength) {
    if (!hasModifiedStrokeHistory &&
        states[lastStrokeOffset].lookupType != SegmentLookupType::AUTO_SUFFIX) {
      return StenoSegment::CreateInvalid();
    }
    startLength = GetStartingDefinitionLength(lastStrokeOffset,
                                              context.maximumOutlineLength);
  }

  size_t minimumLength = offset - lastStrokeOffset + 1;
  return AutoSuffixTest(context, lastStrokeOffset, startLength, minimumLength);
}

StenoSegment StenoSegmentBuilder::AutoSuffixTest(BuildSegmentContext &context,
                                                 size_t offset,
                                                 size_t startLength,
                                                 size_t minimumLength) {
  StenoStroke localStrokes[startLength];
  (strokes + offset)->CopyTo(localStrokes, startLength);

  const StenoOrthography &orthography = context.orthography.data;

  size_t length = startLength;
  while (length >= minimumLength) {
    if ((strokes[offset + length - 1] & orthography.autoSuffixMask)
            .IsNotEmpty()) {
      for (const StenoOrthographyAutoSuffix &suffix :
           orthography.autoSuffixes) {
        if ((strokes[offset + length - 1] & suffix.stroke).IsEmpty()) {
          continue;
        }
        localStrokes[length - 1] =
            strokes[offset + length - 1] & ~suffix.stroke;

        StenoDictionaryLookupResult lookup =
            context.dictionary.Lookup(localStrokes, length);

        if (lookup.IsValid()) {
          const char *text = lookup.GetText();
          const char *result = Str::Join(text, suffix.text);
          lookup.Destroy();
          return StenoSegment(
              length, SegmentLookupType::AUTO_SUFFIX, states + offset,
              StenoDictionaryLookupResult::CreateDynamicString(result));
        }
      }
    }
    if (hasModifiedStrokeHistory ||
        !states[offset + length - 1].IsDefinitionStart()) {
      --length;
    } else if (states[offset].lookupType != SegmentLookupType::AUTO_SUFFIX) {
      break;
    } else {
      length = GetFirstDefinitionBoundaryLength(offset, length);
    }
  }

  return StenoSegment::CreateInvalid();
}

void StenoSegmentBuilder::AddRawStroke(BuildSegmentContext &context,
                                       size_t &offset) {
  hasRawStroke = true;

  char buffer[StenoStroke::MAX_STRING_LENGTH];

  strokes[offset].ToString(buffer);
  context.segmentList.Add(StenoSegment(
      1, SegmentLookupType::STROKE, states + offset,
      StenoDictionaryLookupResult::CreateDynamicString(Str::Dup(buffer))));
  ++offset;
}

void StenoSegmentBuilder::ReevaluateSegments(BuildSegmentContext &context,
                                             size_t &offset) {
  // Removes all segments that could be affected and updates offset to
  // reprocess them.
  size_t currentOffset = offset;
  while (context.segmentList.IsNotEmpty()) {
    StenoSegment &lastSegment = context.segmentList.Back();
    size_t lastOffset = lastSegment.GetStrokeIndex(states);
    if (lastOffset + context.maximumOutlineLength < currentOffset) {
      return;
    }
    lastSegment.lookup.Destroy();
    context.segmentList.Pop();
    offset = lastOffset;
  }

  // Reprocess them.
  AddSegments(context, offset);

  // Ensure currentOffset is tagged with a DIRECT lookup
  for (size_t i = 0; i < context.segmentList.GetCount(); ++i) {
    StenoSegment *segment = &context.segmentList[i];
    size_t segmentOffset = segment->GetStrokeIndex(states);
    const size_t segmentEndOffset = segmentOffset + segment->strokeLength;
    if (segmentEndOffset <= currentOffset) {
      continue;
    }

    if (segmentOffset < currentOffset) {
      const size_t remainingLength = segmentEndOffset - currentOffset;
      // Need to split the segment
      segment->lookupType = SegmentLookupType::DIRECT;
      segment->strokeLength = currentOffset - segmentOffset;
      ++i;
      context.segmentList.InsertAt(
          i, StenoSegment(remainingLength, SegmentLookupType::DIRECT,
                          states + currentOffset,
                          StenoDictionaryLookupResult::CreateStaticString("")));
      segment = &context.segmentList[i];
    }

    if (segment->strokeLength == 1) {
      segment->lookupType = SegmentLookupType::DIRECT;
    } else {
      segment->state++;
      segment->strokeLength--;
      context.segmentList.InsertAt(
          i, StenoSegment(1, SegmentLookupType::DIRECT, states + currentOffset,
                          StenoDictionaryLookupResult::CreateStaticString("")));
    }
    return;
  }
}

struct RetroSetValueParameters {
  int index;
  int wordCount;
  const char *format;

  bool Parse(const char *command);
};

bool RetroSetValueParameters::Parse(const char *command) {
  const char *p = command + Str::Length<>("=set_value:");
  p = Str::ParseInteger(&index, p);
  if (!p || *p != ':') {
    return false;
  }

  p = Str::ParseInteger(&wordCount, p + 1);
  if (!p || wordCount == 0) {
    return false;
  }
  format = p;
  return true;
}

void StenoSegmentBuilder::HandleRetroSetValue(BuildSegmentContext &context,
                                              const char *command,
                                              size_t currentOffset,
                                              size_t length) {
  // set_value:<value_index>:<count>
  // =set_value:0:1
  RetroSetValueParameters parameters;

  if (context.segmentList.GetCount() == 0 || !parameters.Parse(command)) {
    context.segmentList.Add(
        StenoSegment(length, SegmentLookupType::DIRECT, states + currentOffset,
                     StenoDictionaryLookupResult::CreateDynamicString(
                         EscapeCommand(command))));
    return;
  }

  const char *format = parameters.format;
  while (format && *format == ' ') {
    ++format;
  }

  size_t startingSegmentIndex = context.segmentList.GetCount();
  for (size_t i = 0; i < parameters.wordCount && startingSegmentIndex; ++i) {
    startingSegmentIndex = context.segmentList.GetWordStartingSegmentIndex(
        startingSegmentIndex - 1);
  }

  // Only set the value if it is the last entry in the buffer.
  if (context.allowSetValue && currentOffset + length == count) {
    char *result =
        context.engine.ConvertText(context.segmentList, startingSegmentIndex);

    context.engine.SetTemplateValue(parameters.index, Str::Trim(result));
    free(result);
  }

  for (size_t i = startingSegmentIndex; i < context.segmentList.GetCount();
       ++i) {
    StenoSegment &segment = context.segmentList[i];
    segment.lookup.Destroy();
    segment.lookup = StenoDictionaryLookupResult::NO_OP;
  }

  // Use case carry as a no-op on the buffer processing layer.
  // This is so that joinNext state is carried through.
  // Special case if followed by =transform
  StenoDictionaryLookupResult lookup =
      StenoDictionaryLookupResult::CreateInvalid();

  BufferWriter writer;
  EscapeCommand(writer, command);
  if (Str::HasPrefix(format, "=transform:")) {
    CreateTransformString(writer, context,
                          format + Str::Length<>("=transform:"));
  } else {
    writer.WriteString(format);
  }
  lookup = StenoDictionaryLookupResult::CreateDynamicString(
      writer.TerminateStringAndAdoptBuffer());

  hasModifiedStrokeHistory = true;
  states[currentOffset].joinNext =
      context.segmentList[startingSegmentIndex].state->joinNext;
  context.segmentList.Add(StenoSegment(length, SegmentLookupType::DIRECT,
                                       states + currentOffset, lookup));
}

void StenoSegmentBuilder::HandleRetroTransform(BuildSegmentContext &context,
                                               const char *command,
                                               size_t currentOffset,
                                               size_t length) {

  const char *format = command + Str::Length<>("=retro_transform:");

  size_t count = 0;
  if (Unicode::IsAsciiDigit(*format)) {
    count = *format++ - '0';
    while (Unicode::IsAsciiDigit(*format)) {
      count = 10 * count + *format++ - '0';
    }
    if (*format == ':') {
      ++format;
    }
  } else {
    count = 1;
  }

  size_t startingSegmentIndex = context.segmentList.GetCount();
  for (size_t i = 0; i < count && startingSegmentIndex; ++i) {
    startingSegmentIndex = context.segmentList.GetWordStartingSegmentIndex(
        startingSegmentIndex - 1);
  }

  BufferWriter bufferWriter;
  EscapeCommand(bufferWriter, command);
  WriteRetroTransform(context.segmentList, startingSegmentIndex, format,
                      bufferWriter);

  for (size_t i = startingSegmentIndex; i < context.segmentList.GetCount();
       ++i) {
    StenoSegment &segment = context.segmentList[i];
    segment.lookup.Destroy();
    segment.lookup = StenoDictionaryLookupResult::NO_OP;
  }

  hasModifiedStrokeHistory = true;
  context.segmentList.Add(
      StenoSegment(length, SegmentLookupType::DIRECT, states + currentOffset,
                   StenoDictionaryLookupResult::CreateDynamicString(
                       bufferWriter.TerminateStringAndAdoptBuffer())));
}

void StenoSegmentBuilder::WriteRetroTransform(const StenoSegmentList &segments,
                                              size_t startingSegmentIndex,
                                              const char *format,
                                              BufferWriter &output) const {

  const size_t strokeCount =
      segments.Back().GetEndStenoState() - segments[startingSegmentIndex].state;

  for (;;) {
    switch (char c = *format++; c) {
    case '\0':
      return;
    case '%':
      c = *format++;
      switch (c) {
      case '\0':
        return;
      case 'd':
      case 'l':
        for (size_t i = startingSegmentIndex; i < segments.GetCount(); ++i) {
          if (i != startingSegmentIndex) {
            output.WriteByte(' ');
          }
          const char *text = segments[i].lookup.GetText();
          // Need to escape any special characters.
          while (*text) {
            switch (*text) {
            case '{':
            case ' ':
            case '\\':
              output.WriteByte('\\');
              [[fallthrough]];
            default:
              output.WriteByte(*text++);
            }
          }
        }
        break;
      case 'o':
      case 's': {
        char buffer[StenoStroke::MAX_STRING_LENGTH];
        const StenoStroke *strokes =
            this->strokes +
            segments[startingSegmentIndex].GetStrokeIndex(states);

        for (size_t i = 0; i < strokeCount; ++i) {
          if (i != 0) {
            output.WriteByte('/');
          }
          char *bufferEnd = strokes[i].ToString(buffer);
          output.Write(buffer, bufferEnd - buffer);
        }
      } break;
      case 't':
        output.Write("{^}", 3);
        for (size_t i = startingSegmentIndex; i < segments.GetCount(); ++i) {
          output.WriteByte(' ');
          const char *text = segments[i].lookup.GetText();
          output.WriteString(text);
        }
        output.Write("{^}", 3);
        break;
      case '%':
        output.WriteByte('%');
        break;
      default:
        output.WriteByte('%');
        output.WriteByte(c);
        break;
      }
      break;
    default:
      output.WriteByte(c);
      break;
    }
  }
}

void StenoSegmentBuilder::CreateTransformString(BufferWriter &bufferWriter,
                                                BuildSegmentContext &context,
                                                const char *format) const {

  enum class Formatter {
    NORMAL,
    UPPER,
    LOWER,
    TITLE,
    CAPITALIZE,
  };

  Formatter formatter;
  Utf8Pointer p(format);

  for (;;) {
    switch (int c = *p++; c) {
    case '\0':
      return;

    case '%':
      c = *p++;
      formatter = Formatter::NORMAL;

      switch (c) {
      case '\0':
        return;
      case 'u':
        formatter = Formatter::UPPER;
        c = *p++;
        break;
      case 'l':
        formatter = Formatter::LOWER;
        c = *p++;
        break;
      case 't':
        formatter = Formatter::TITLE;
        c = *p++;
        break;
      case 'c':
        formatter = Formatter::CAPITALIZE;
        c = *p++;
        break;
      }

      switch (c) {
      case '%':
        bufferWriter.WriteByte('%');
        break;
      case '0' ... '9': {
        size_t index = c - '0';
        while (Unicode::IsAsciiDigit(*p)) {
          index = index * 10 + *p++ - '0';
        }
        const char *value = context.engine.GetTemplateValue(index);

        bool isStart = true;
        while (*value) {
          uint32_t c = *value++;

          switch (formatter) {
          case Formatter::NORMAL:
            // Do nothing
            break;
          case Formatter::UPPER:
            c = Unicode::ToUpper(c);
            break;
          case Formatter::LOWER:
            c = Unicode::ToLower(c);
            break;
          case Formatter::TITLE:
            if (Unicode::IsWhitespace(c)) {
              isStart = true;
            } else if (isStart) {
              c = Unicode::ToUpper(c);
              isStart = false;
            }
            break;
          case Formatter::CAPITALIZE:
            if (!Unicode::IsWhitespace(c) && isStart) {
              c = Unicode::ToUpper(c);
              isStart = false;
            }
            break;
          }

          if (c < 128) {
            switch (c) {
            case ' ':
            case '\\':
            case '{':
              bufferWriter.WriteByte('\\');
              [[fallthrough]];

            default:
              bufferWriter.WriteByte(c);
              break;
            }
          } else {
            char data[4];
            Utf8Pointer output(data);
            output.SetAndAdvance(c);
            bufferWriter.Write(data, output.GetRawPointer() - data);
          }
        }
      } break;
      default:
        bufferWriter.WriteByte('%');
        bufferWriter.WriteByte(c);
        break;
      }
      break;
    default:
      bufferWriter.WriteByte(c);
      break;
    }
  }
}

char *StenoSegmentBuilder::EscapeCommand(const char *p) {
  BufferWriter writer;
  EscapeCommand(writer, p);
  return writer.TerminateStringAndAdoptBuffer();
}

void StenoSegmentBuilder::EscapeCommand(BufferWriter &writer, const char *p) {
  writer.WriteByte('{');
  writer.WriteByte(':');
  writer.WriteByte('=');
  while (*p) {
    int c = *p++;
    switch (c) {
    case '{':
    case '}':
    case ':':
    case '\\':
      writer.WriteByte('\\');
      [[fallthrough]];
    default:
      writer.WriteByte(c);
      break;
    }
  }
  writer.WriteByte('}');
}

void StenoSegmentBuilder::UpdateLastSegmentWithCommand(
    BuildSegmentContext &context, const char *command) {
  if (context.segmentList.IsEmpty()) {
    return;
  }

  StenoSegment &back = context.segmentList.Back();
  BufferWriter writer;
  EscapeCommand(writer, command);
  const char *text = back.lookup.GetText();
  writer.WriteString(text);
  back.lookup.Destroy();
  back.lookup = StenoDictionaryLookupResult::CreateDynamicString(
      writer.TerminateStringAndAdoptBuffer());
}

void StenoSegmentBuilder::ResetStrokes(size_t offset, size_t length) {
  StenoState emptyState;
  emptyState.Reset();

  for (size_t i = 0; i < length; ++i) {
    strokes[offset + i] = StenoStroke(0);
    states[offset + i] = emptyState;
  }
}

void StenoSegmentBuilder::HandleRetroInsertSpace(BuildSegmentContext &context,
                                                 size_t currentOffset,
                                                 size_t length) {
  hasModifiedStrokeHistory = true;

  if (currentOffset == 0) {
    ResetStrokes(currentOffset, length);
    return;
  }

  StenoStroke lastStroke = strokes[currentOffset - 1];
  StenoState lastState = states[currentOffset - 1];

  ResetStrokes(currentOffset - 1, length);

  strokes[currentOffset + length - 1] = lastStroke;
  states[currentOffset + length - 1] = lastState;
}

void StenoSegmentBuilder::HandleRetroToggleAsterisk(
    BuildSegmentContext &context, size_t currentOffset, size_t length) {
  hasModifiedStrokeHistory = true;

  ResetStrokes(currentOffset, length);

  if (currentOffset == 0) {
    return;
  }

  strokes[currentOffset - 1] ^= StrokeMask::STAR;
}

void StenoSegmentBuilder::HandleRepeatLastStroke(BuildSegmentContext &context,
                                                 size_t currentOffset,
                                                 size_t length) {
  hasModifiedStrokeHistory = true;

  if (currentOffset == 0) {
    ResetStrokes(currentOffset, length);
    return;
  }

  StenoStroke lastStroke = strokes[currentOffset - 1];
  StenoState state = states[currentOffset];

  ResetStrokes(currentOffset, length - 1);

  strokes[currentOffset + length - 1] = lastStroke;
  states[currentOffset + length - 1] = state;
}

//---------------------------------------------------------------------------

#include "dictionary/compact_map_dictionary.h"
#include "dictionary/debug_dictionary.h"
#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/test_dictionary.h"
#include "engine.h"
#include "orthography.h"
#include "str.h"
#include "unit_test.h"

static StenoCompactMapDictionary mainDictionary(TestDictionary::definition);

StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

TEST_BEGIN("StrokeHistory: Test single segment") {
  StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  StenoEngine engine(dictionary, orthography);
  BuildSegmentContext context(segmentList, engine, false);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test two segments, with multi-stroke") {
  StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState{});
  history.Add(StenoStroke("TEFT"), StenoState{});
  history.Add(StenoStroke("-D"), StenoState{});
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  StenoEngine engine(dictionary, orthography);
  BuildSegmentContext context(segmentList, engine, false);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "tested"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test *? splits strokes") {
  StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState{});
  history.Add(StenoStroke("-D"), StenoState{});
  history.Add(StenoStroke("SKWHU"), StenoState{});
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  StenoEngine engine(dictionary, orthography);
  BuildSegmentContext context(segmentList, engine, false);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 3);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), ""));
  assert(Str::Eq(segmentList[2].lookup.GetText(), "{:=\\{*?\\}}-D"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test * toggles lookup") {
  StenoDebugDictionary dictionary;
  dictionary.SetResponse("{*}");

  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("#EU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  StenoEngine engine(dictionary, orthography);
  BuildSegmentContext context(segmentList, engine, false);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "T*EFT"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "{:=\\{*\\}}"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test {*?} behaves properly") {
  StenoSegmentBuilder history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("SKWH-U"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  StenoDictionaryList dictionary(DICTIONARIES, 2);
  StenoEngine engine(dictionary, orthography);
  BuildSegmentContext context(segmentList, engine, false);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 10);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), ""));
  assert(Str::Eq(segmentList[2].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[3].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[4].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[5].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[6].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[7].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[8].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[9].lookup.GetText(), "test"));
}
TEST_END

//---------------------------------------------------------------------------
