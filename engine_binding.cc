//---------------------------------------------------------------------------

#include "engine.h"

#include "console.h"

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

  StenoEngine *engine = (StenoEngine *)context;
  if (engine->EnableDictionary(dictionary)) {
    Console::SendOk();
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
    Console::SendOk();
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
    Console::SendOk();
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
  StenoEngine *engine = (StenoEngine *)context;
  const char *lookup = strchr(commandLine, ' ');
  if (lookup == nullptr) {
    Console::Printf("ERR Unable to lookup empty word\n\n");
    return;
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
