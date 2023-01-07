//---------------------------------------------------------------------------

#include "engine.h"

#include "console.h"

//---------------------------------------------------------------------------

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

  StenoEngine *engine = (StenoEngine *)context;
  if (engine->EnableDictionary(dictionary)) {
    Console::Write("OK\n\n", 4);
  } else {
    Console::Printf("ERR Unable to enable dictionary: \"%s\"\n\n", dictionary);
  }
}

void StenoEngine::DisableDictionary_Binding(void *context,
                                            const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (!dictionary) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }
  ++dictionary;

  StenoEngine *engine = (StenoEngine *)context;
  if (engine->DisableDictionary(dictionary)) {
    Console::Write("OK\n\n", 4);
  } else {
    Console::Printf("ERR Unable to disable dictionary: \"%s\"\n\n", dictionary);
  }
}

void StenoEngine::ToggleDictionary_Binding(void *context,
                                           const char *commandLine) {
  const char *dictionary = strchr(commandLine, ' ');
  if (!dictionary) {
    Console::Printf("ERR No dictionary specified\n\n");
    return;
  }
  ++dictionary;

  StenoEngine *engine = (StenoEngine *)context;
  if (engine->ToggleDictionary(dictionary)) {
    Console::Write("OK\n\n", 4);
  } else {
    Console::Printf("ERR Unable to toggle dictionary: \"%s\"\n\n", dictionary);
  }
}

void StenoEngine::PrintDictionary_Binding(void *context,
                                          const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->PrintDictionary();
}

void StenoEngine::EnablePaperTape_Binding(void *context,
                                          const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->EnablePaperTape();
  Console::Write("OK\n\n", 4);
}

void StenoEngine::DisablePaperTape_Binding(void *context,
                                           const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->DisablePaperTape();
  Console::Write("OK\n\n", 4);
}

void StenoEngine::EnableSuggestions_Binding(void *context,
                                            const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->EnableSuggestions();
  Console::Write("OK\n\n", 4);
}

void StenoEngine::DisableSuggestions_Binding(void *context,
                                             const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->DisableSuggestions();
  Console::Write("OK\n\n", 4);
}

void StenoEngine::Lookup_Binding(void *context, const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  const char *lookup = strchr(commandLine, ' ');
  if (lookup == nullptr) {
    Console::Printf("ERR Unable to lookup empty word\n\n");
  }

  ++lookup;
  StenoReverseDictionaryLookup result(
      StenoReverseDictionaryLookup::MAX_STROKE_THRESHOLD, lookup);
  engine->ReverseLookup(result);

  char buffer[256];
  Console::Printf("[");
  for (size_t i = 0; i < result.resultCount; ++i) {
    const StenoReverseDictionaryResult lookup = result.results[i];

    StenoStroke::ToString(lookup.strokes, lookup.length, buffer);
    Console::Printf(i == 0 ? "\n  \"%s\"" : ",\n  \"%s\"", buffer);
  }

  Console::Write("\n]\n\n", 4);
}

//---------------------------------------------------------------------------
