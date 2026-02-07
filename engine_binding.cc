//---------------------------------------------------------------------------

#include "console.h"
#include "dictionary/dictionary.h"
#include "engine.h"
#include "hal/external_flash.h"
#include "stroke_list_parser.h"
#include "writer.h"

//---------------------------------------------------------------------------

void StenoEngine::CreateSegments(StenoSegmentList &segments,
                                 StenoSegmentBuilder &segmentBuilder,
                                 const StenoStroke *strokes, size_t length) {
  segmentBuilder.Reset();
  segmentBuilder.Add(strokes, length);

  BuildSegmentContext context(segments, *this);
  segmentBuilder.CreateSegments(context);
}

//---------------------------------------------------------------------------

void StenoEngine::SetSpacePosition_Binding(void *context,
                                           const char *commandLine) {
  const char *stenoMode = strchr(commandLine, ' ');
  if (!stenoMode) {
    Console::Printf("ERR No space position specified\n\n");
    return;
  }

  ++stenoMode;
  if (Str::Eq(stenoMode, "before")) {
    ((StenoEngine *)context)->placeSpaceAfter = false;
  } else if (Str::Eq(stenoMode, "after")) {
    ((StenoEngine *)context)->placeSpaceAfter = true;
  } else {
    Console::Printf("ERR Unable to set space position: \"%s\"\n\n", stenoMode);
    return;
  }

  Console::SendOk();
}

void StenoEngine::ListDictionaries_Binding(void *context,
                                           const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->ListDictionaries();
}

void StenoEngine::EnableDictionary_Binding(void *context,
                                           const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (!dictionary) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }
  ++dictionary;

  BufferWriter bufferWriter;
  ConsoleWriter::Push(&bufferWriter);
  StenoEngine *engine = (StenoEngine *)context;
  const bool result = engine->EnableDictionary(dictionary);
  ConsoleWriter::Pop();
  if (result) {
    Console::SendOk();
  } else {
    Console::Printf("ERR Unable to enable dictionary: \"%s\"\n\n", dictionary);
  }
  bufferWriter.WriteBufferTo(ConsoleWriter::GetActiveWriter());
}

void StenoEngine::DisableDictionary_Binding(void *context,
                                            const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (!dictionary) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }
  ++dictionary;

  BufferWriter bufferWriter;
  ConsoleWriter::Push(&bufferWriter);
  StenoEngine *engine = (StenoEngine *)context;
  const bool result = engine->DisableDictionary(dictionary);
  ConsoleWriter::Pop();
  if (result) {
    Console::SendOk();
  } else {
    Console::Printf("ERR Unable to disable dictionary: \"%s\"\n\n", dictionary);
  }
  bufferWriter.WriteBufferTo(ConsoleWriter::GetActiveWriter());
}

void StenoEngine::ToggleDictionary_Binding(void *context,
                                           const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (!dictionary) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }
  ++dictionary;

  BufferWriter bufferWriter;
  ConsoleWriter::Push(&bufferWriter);
  StenoEngine *engine = (StenoEngine *)context;
  const bool result = engine->ToggleDictionary(dictionary);
  ConsoleWriter::Pop();
  if (result) {
    Console::SendOk();
  } else {
    Console::Printf("ERR Unable to toggle dictionary: \"%s\"\n\n", dictionary);
  }
  bufferWriter.WriteBufferTo(ConsoleWriter::GetActiveWriter());
}

void StenoEngine::PrintDictionary_Binding(void *context,
                                          const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (dictionary) {
    ++dictionary;
  }

  StenoEngine *engine = (StenoEngine *)context;
  engine->PrintDictionary(dictionary);
}

void StenoEngine::Lookup_Binding(void *context, const char *commandLine) {
  const char *definition = strchr(commandLine, ' ');
  if (definition == nullptr) {
    Console::Printf("ERR Unable to lookup empty word\n\n");
    return;
  }

  const ExternalFlashSentry externalFlashSentry;

  ++definition;
  StenoReverseDictionaryLookup lookup(definition);
  StenoEngine *engine = (StenoEngine *)context;
  engine->ReverseLookup(lookup);

  Console::Printf("[");

  ConsoleLookupDictionaryContext lookupDictionaryContext(definition);

  for (const StenoReverseDictionaryResult &entry : lookup.results) {
    StenoSegmentList segments(entry.length);
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segments, buffer.segmentBuilder, entry.strokes,
                           entry.length);

    // Print definition.
    if (segments.GetCount() == 1) {
      char *t = Str::Trim(segments[0].lookup.GetText());
      lookupDictionaryContext.Add(entry.strokes, entry.length, t,
                                  entry.dictionary);
      free(t);
    } else {
      BufferWriter writer;
      for (const StenoSegment &segment : segments) {
        if (writer.IsNotEmpty()) {
          writer.WriteByte(' ');
        }
        writer.WriteString(segment.lookup.GetText());
      }
      writer.WriteByte(0);
      lookupDictionaryContext.Add(entry.strokes, entry.length,
                                  writer.GetBuffer(), entry.dictionary);
    }
  }

  Console::Printf("]\n\n");
}

void StenoEngine::LookupPrefix_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No count specified\n\n");
    return;
  }

  int count = 0;
  p = Str::ParseInteger(&count, p + 1, false);
  if (!p) {
    Console::Printf("ERR No count specified\n\n");
    return;
  }

  if (!*p) {
    Console::Printf("ERR No text specified\n\n");
    return;
  }

  const ExternalFlashSentry externalFlashSentry;

  Console::Printf("[");
  PrintPrefixContext lookupContext(p + 1, size_t(count));

  StenoEngine *engine = (StenoEngine *)context;
  engine->GetDictionary().PrintEntriesWithPrefix(lookupContext);
  Console::Printf("]\n\n");
}

void StenoEngine::LookupStroke_Binding(void *context, const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No strokes specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  const ExternalFlashSentry externalFlashSentry;
  StenoEngine *engine = (StenoEngine *)context;
  List<const StenoDictionary *> dictionaries;
  engine->GetDictionary().GetDictionariesForOutline(
      dictionaries, parser.GetData(), parser.GetCount());

  if (dictionaries.IsNotEmpty()) {
    Console::Printf("[");
    bool isFirstTime = true;
    for (const StenoDictionary *dictionary : dictionaries) {
      const StenoDictionaryLookupResult result =
          dictionary->Lookup(parser.GetData(), parser.GetCount());

      const char *format = ",{t: %Y,d: %Y%s}";
      Console::Printf(format + isFirstTime, result.GetText(),
                      dictionary->GetName(),
                      dictionary->CanRemove() ? ",r: 1" : "");

      isFirstTime = false;
    }
    Console::Printf("]\n\n");
  } else {
    StenoSegmentList segments(parser.GetCount());
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segments, buffer.segmentBuilder, parser.GetData(),
                           parser.GetCount());

    if (!buffer.segmentBuilder.HasRawStroke()) {
      char scratchBuffer[256];
      BufferWriter buffer(scratchBuffer, sizeof(scratchBuffer));

      buffer.WriteString(segments[0].lookup.GetText());
      for (const StenoSegment &segment : segments.Skip(1)) {
        buffer.WriteByte(' ');
        buffer.WriteString(segment.lookup.GetText());
      }
      buffer.WriteByte('\0');
      Console::Printf("t: %Y\n\n", buffer.GetBuffer());
    } else {
      Console::Printf("[]\n\n");
    }
  }
}

void StenoEngine::LookupPartialOutline_Binding(void *context,
                                               const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No strokes specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  int count = 0;
  const char *p = parser.failureOrEnd;
  if (*p != '\0') {
    Str::ParseInteger(&count, p, false);
  }

  const ExternalFlashSentry externalFlashSentry;

  Console::Printf("[");
  PrintPartialOutlineContext lookupContext(parser.GetData(), parser.GetCount(),
                                           size_t(count));
  StenoEngine *engine = (StenoEngine *)context;
  engine->GetDictionary().PrintEntriesWithPartialOutline(lookupContext);
  Console::Printf("]\n\n");
}

void StenoEngine::RemoveOutline_Binding(void *context,
                                        const char *commandLine) {
  const char *dictionaryStart = strchr(commandLine, ' ');
  const char *strokeStart = strrchr(commandLine, ' ');
  if (!dictionaryStart || dictionaryStart == strokeStart) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  char *dictionaryName =
      Str::DupN(dictionaryStart + 1, strokeStart - dictionaryStart - 1);

  const ExternalFlashSentry externalFlashSentry;
  StenoEngine *engine = (StenoEngine *)context;
  const bool result = engine->GetDictionary().Remove(
      dictionaryName, parser.GetData(), parser.GetCount());

  if (!result) {
    Console::Printf("ERR Unable to delete stroke %s from dictionary %s\n\n",
                    strokeStart + 1, dictionaryName);
  } else {
    Console::SendOk();
  }
  free(dictionaryName);
}

void StenoEngine::ProcessStrokes_Binding(void *context,
                                         const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No strokes specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  Console::SendOk();

  ConsoleWriter::Push(&ConsoleWriter::instance);
  StenoEngine *engine = (StenoEngine *)context;
  for (const StenoStroke &stroke : parser) {
    engine->Process(stroke);
  }
  ConsoleWriter::Pop();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-plus-int"
void StenoEngine::ListTemplateValues_Binding(void *context,
                                             const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;

  Console::Printf("[");
  size_t emptyCount = 0;
  bool isFirst = true;
  for (size_t i = 0; i < TEMPLATE_VALUE_COUNT; ++i) {
    const char *value = engine->templateValues[i].GetValue();
    if (*value == '\0') {
      emptyCount++;
      continue;
    }
    if (emptyCount) {
      Console::Printf(",%d" + isFirst, emptyCount);
      emptyCount = 0;
      isFirst = false;
    }
    Console::Printf(",%Y" + isFirst, value);
    isFirst = false;
  }
  if (emptyCount) {
    Console::Printf(",%d" + isFirst, emptyCount);
  }
  Console::Printf("]\n\n");
}
#pragma clang diagnostic pop

void StenoEngine::SetTemplateValue_Binding(void *context,
                                           const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No parameters specified\n\n");
    return;
  }
  int index = 0;
  ++p;
  p = Str::ParseInteger(&index, p, false);
  if (!p) {
    Console::Printf("ERR index parameter missing\n\n");
    return;
  }
  if (index >= TEMPLATE_VALUE_COUNT) {
    Console::Printf("ERR index parameter out of range\n\n");
    return;
  }
  if (*p != ' ') {
    Console::Printf("ERR value missing\n\n");
    return;
  }
  ++p;

  Console::SendOk();

  StenoEngine *engine = (StenoEngine *)context;
  engine->SetTemplateValue(index, Str::Trim(p));
}

//---------------------------------------------------------------------------

void StenoEngine::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_space_position",
                          "Controls space position [\"before\", \"after\"]",
                          StenoEngine::SetSpacePosition_Binding, this);
  console.RegisterCommand("list_dictionaries", "Lists dictionaries",
                          StenoEngine::ListDictionaries_Binding, this);
  console.RegisterCommand("enable_dictionary", "Enables a dictionary",
                          StenoEngine::EnableDictionary_Binding, this);
  console.RegisterCommand("disable_dictionary", "Disables a dictionary",
                          StenoEngine::DisableDictionary_Binding, this);
  console.RegisterCommand("toggle_dictionary", "Toggles a dictionary",
                          StenoEngine::ToggleDictionary_Binding, this);
  console.RegisterCommand("print_dictionary",
                          "Prints all dictionaries in JSON format",
                          StenoEngine::PrintDictionary_Binding, this);
  console.RegisterCommand("lookup", "Looks up a definition",
                          StenoEngine::Lookup_Binding, this);
  console.RegisterCommand("lookup_prefix",
                          "Looks up entries with the given prefix",
                          StenoEngine::LookupPrefix_Binding, this);
  console.RegisterCommand("lookup_stroke", "Looks up an outline",
                          StenoEngine::LookupStroke_Binding, this);
  console.RegisterCommand("lookup_partial_outline",
                          "Looks up entries which contain the given outline",
                          StenoEngine::LookupPartialOutline_Binding, this);
  console.RegisterCommand("remove_outline",
                          "Removes an outline from specified dictionary",
                          StenoEngine::RemoveOutline_Binding, this);
  console.RegisterCommand("process_strokes", "Processes a stroke list",
                          StenoEngine::ProcessStrokes_Binding, this);
  console.RegisterCommand("list_template_values", "Lists all template values",
                          StenoEngine::ListTemplateValues_Binding, this);
  console.RegisterCommand("set_template_value", "Sets template value",
                          StenoEngine::SetTemplateValue_Binding, this);
}

//---------------------------------------------------------------------------
