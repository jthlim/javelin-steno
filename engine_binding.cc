//---------------------------------------------------------------------------

#include "console.h"
#include "engine.h"
#include "hal/external_flash.h"
#include "stroke_list_parser.h"

//---------------------------------------------------------------------------

void StenoEngine::CreateSegments(StenoSegmentList &segmentList,
                                 StenoSegmentBuilder &segmentBuilder,
                                 const StenoStroke *strokes, size_t length) {
  segmentBuilder.Reset();
  segmentBuilder.Add(strokes, length);

  BuildSegmentContext context(segmentList, dictionary, orthography);
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

void StenoEngine::EnablePaperTape_Binding(void *context,
                                          const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->EnablePaperTape();
  Console::SendOk();
}

void StenoEngine::DisablePaperTape_Binding(void *context,
                                           const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->DisablePaperTape();
  Console::SendOk();
}

void StenoEngine::EnableSuggestions_Binding(void *context,
                                            const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->EnableSuggestions();
  Console::SendOk();
}

void StenoEngine::DisableSuggestions_Binding(void *context,
                                             const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->DisableSuggestions();
  Console::SendOk();
}

void StenoEngine::EnableTextLog_Binding(void *context,
                                        const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->EnableTextLog();
  Console::SendOk();
}

void StenoEngine::DisableTextLog_Binding(void *context,
                                         const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->DisableTextLog();
  Console::SendOk();
}

void StenoEngine::Lookup_Binding(void *context, const char *commandLine) {
  const char *definition = strchr(commandLine, ' ');
  if (definition == nullptr) {
    Console::Printf("ERR Unable to lookup empty word\n\n");
    return;
  }

  const ExternalFlashSentry externalFlashSentry;

  ++definition;
  StenoReverseDictionaryLookup lookup(
      StenoReverseDictionaryLookup::MAX_STROKE_THRESHOLD, definition);
  StenoEngine *engine = (StenoEngine *)context;
  engine->ReverseLookup(lookup);

  Console::Printf("[");
  for (const StenoReverseDictionaryResult &entry : lookup.results) {
    StenoSegmentList segmentList;
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segmentList, buffer.segmentBuilder, entry.strokes,
                           entry.length);

    Console::Printf(&entry == begin(lookup.results) ? "\n{" : ",\n{", nullptr);
    Console::Printf("\"outline\":\"%T\"", entry.strokes, entry.length);
    Console::Printf(",\"definition\":\"");

    bool isFirst = true;
    StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
    while (tokenizer->HasMore()) {
      Console::Printf(isFirst ? "%J" : " %J", tokenizer->GetNext().text);
      isFirst = false;
    }
    delete tokenizer;
    Console::Printf("\"");

    const char *name = entry.dictionary->GetName();
    if (*name == '#') {
      Console::Printf(",\"dictionary\":\"#\"");
    } else {
      Console::Printf(",\"dictionary\":\"%J\"", name);
      if (entry.dictionary->CanRemove()) {
        Console::Printf(",\"can_remove\":true");
      }
    }

    Console::Printf("}");
  }

  Console::Printf("\n]\n\n");
}

void StenoEngine::LookupStroke_Binding(void *context, const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No stroke specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  const ExternalFlashSentry externalFlashSentry;
  StenoEngine *engine = (StenoEngine *)context;
  const StenoDictionaryLookupResult result =
      engine->dictionary.Lookup(parser.strokes, parser.length);

  if (result.IsValid()) {
    const StenoDictionary *dictionary =
        engine->dictionary.GetDictionaryForOutline(parser.strokes,
                                                   parser.length);
    Console::Printf("{\"definition\":\"%J\",\"dictionary\":\"%J\"",
                    result.GetText(), dictionary->GetName());

    if (dictionary->CanRemove()) {
      Console::Printf(",\"can_remove\":true");
    }

    Console::Printf("}\n\n");
  } else {
    StenoSegmentList segmentList;
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segmentList, buffer.segmentBuilder, parser.strokes,
                           parser.length);

    if (!buffer.segmentBuilder.HasRawStroke()) {
      Console::Printf("{\"definition\":\"");

      bool isFirst = true;
      StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
      while (tokenizer->HasMore()) {
        Console::Printf(isFirst ? "%J" : " %J", tokenizer->GetNext().text);
        isFirst = false;
      }
      delete tokenizer;
      Console::Printf("\",\"dictionary\":\"#\"}\n\n");
    } else {
      Console::Printf("null\n\n");
    }
  }
}

void StenoEngine::RemoveStroke_Binding(void *context, const char *commandLine) {
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
  bool result =
      engine->dictionary.Remove(dictionaryName, parser.strokes, parser.length);

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
    Console::Printf("ERR No stroke specified\n\n");
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
  for (size_t i = 0; i < parser.length; ++i) {
    engine->ProcessStroke(parser.strokes[i]);
  }
  ConsoleWriter::Pop();
}

//---------------------------------------------------------------------------
