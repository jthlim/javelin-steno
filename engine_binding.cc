//---------------------------------------------------------------------------

#include "console.h"
#include "engine.h"
#include "hal/external_flash.h"
#include "stroke_list_parser.h"

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
  bool result = engine->EnableDictionary(dictionary);
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
  bool result = engine->DisableDictionary(dictionary);
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
  bool result = engine->ToggleDictionary(dictionary);
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
  const char *lookup = strchr(commandLine, ' ');
  if (lookup == nullptr) {
    Console::Printf("ERR Unable to lookup empty word\n\n");
    return;
  }

  ++lookup;
  StenoReverseDictionaryLookup result(
      StenoReverseDictionaryLookup::MAX_STROKE_THRESHOLD, lookup);
  StenoEngine *engine = (StenoEngine *)context;
  engine->ReverseLookup(result);

  char buffer[256];
  Console::Printf("[");
  for (size_t i = 0; i < result.resultCount; ++i) {
    const StenoReverseDictionaryResult lookup = result.results[i];

    StenoStroke::ToString(buffer, lookup.strokes, lookup.length);
    Console::Printf(i == 0 ? "\n  \"%s\"" : ",\n  \"%s\"", buffer);
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

  ExternalFlashSentry externalFlashSentry;
  StenoEngine *engine = (StenoEngine *)context;
  StenoDictionaryLookupResult result =
      engine->dictionary.Lookup(parser.strokes, parser.length);

  if (result.IsValid()) {
    Console::Printf("\"");
    Console::WriteAsJson(result.GetText());
    Console::Printf("\"\n\n");
  } else {
    Console::Printf("null\n\n");
  }
}

//---------------------------------------------------------------------------
