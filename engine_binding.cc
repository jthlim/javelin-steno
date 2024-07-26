//---------------------------------------------------------------------------

#include "console.h"
#include "dictionary/dictionary_list.h"
#include "engine.h"
#include "hal/external_flash.h"
#include "stroke_list_parser.h"

//---------------------------------------------------------------------------

void StenoEngine::CreateSegments(StenoSegmentList &segments,
                                 StenoSegmentBuilder &segmentBuilder,
                                 const StenoStroke *strokes, size_t length) {
  segmentBuilder.Reset();
  segmentBuilder.Add(strokes, length);

  BuildSegmentContext context(segments, *this, false);
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
    StenoSegmentList segments;
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segments, buffer.segmentBuilder, entry.strokes,
                           entry.length);

    Console::Printf(&entry == begin(lookup.results) ? "\n{" : ",\n{", nullptr);
    Console::Printf("\"outline\":\"%T\"", entry.strokes, entry.length);
    Console::Printf(",\"definition\":\"");

    bool isFirst = true;
    StenoTokenizer *tokenizer = StenoTokenizer::Create(segments);
    while (tokenizer->HasMore()) {
      Console::Printf(isFirst ? "%J" : " %J", tokenizer->GetNext().text);
      isFirst = false;
    }
    delete tokenizer;
    Console::Printf("\"");

    const char *name = entry.dictionary->GetName();
    if (*name != '#') {
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
    StenoSegmentList segments;
    ConversionBuffer &buffer = engine->previousConversionBuffer;
    engine->CreateSegments(segments, buffer.segmentBuilder, parser.strokes,
                           parser.length);

    if (!buffer.segmentBuilder.HasRawStroke()) {
      Console::Printf("{\"definition\":\"");

      bool isFirst = true;
      StenoTokenizer *tokenizer = StenoTokenizer::Create(segments);
      while (tokenizer->HasMore()) {
        Console::Printf(isFirst ? "%J" : " %J", tokenizer->GetNext().text);
        isFirst = false;
      }
      delete tokenizer;
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

void StenoEngine::ListTemplateValues_Binding(void *context,
                                             const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;

  Console::Printf("[");
  for (size_t i = 0; i < TEMPLATE_VALUE_COUNT; ++i) {
    Console::Printf(i == 0 ? "\n  \"%J\"" : ",\n  \"%J\"",
                    engine->templateValues[i].GetValue());
  }
  Console::Printf("\n]\n\n");
}

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

  StenoEngine *engine = (StenoEngine *)context;
  engine->SetTemplateValue(index, Str::Trim(p));
  Console::SendOk();
}

void StenoEngine::EnableTemplateValueUpdate_Binding(void *context,
                                                    const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->templateValueUpdateEnabled = true;
  Console::SendOk();
}

void StenoEngine::DisableTemplateValueUpdate_Binding(void *context,
                                                     const char *commandLine) {
  StenoEngine *engine = (StenoEngine *)context;
  engine->templateValueUpdateEnabled = false;
  Console::SendOk();
}

//---------------------------------------------------------------------------

void StenoEngine::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_space_position",
                          "Controls space position before or after",
                          StenoEngine::SetSpacePosition_Binding, this);
  console.RegisterCommand("list_dictionaries", "Lists dictionaries",
                          StenoEngine::ListDictionaries_Binding, this);
  console.RegisterCommand("enable_dictionary", "Enables a dictionary",
                          StenoEngine::EnableDictionary_Binding, this);
  console.RegisterCommand("disable_dictionary", "Disable a dictionary",
                          StenoEngine::DisableDictionary_Binding, this);
  console.RegisterCommand("toggle_dictionary", "Toggle a dictionary",
                          StenoEngine::ToggleDictionary_Binding, this);
  console.RegisterCommand("print_dictionary",
                          "Prints all dictionaries in JSON format",
                          StenoEngine::PrintDictionary_Binding, this);
  console.RegisterCommand(
      "enable_dictionary_status", "Enable sending dictionary status updates",
      StenoDictionaryList::EnableDictionaryStatus_Binding, nullptr);
  console.RegisterCommand(
      "disable_dictionary_status", "Disable sending dictionary status updates",
      StenoDictionaryList::DisableDictionaryStatus_Binding, nullptr);
  console.RegisterCommand("enable_paper_tape", "Enables paper tape output",
                          StenoEngine::EnablePaperTape_Binding, this);
  console.RegisterCommand("disable_paper_tape", "Disables paper tape output",
                          StenoEngine::DisablePaperTape_Binding, this);
  console.RegisterCommand("enable_suggestions", "Enables suggestions output",
                          StenoEngine::EnableSuggestions_Binding, this);
  console.RegisterCommand("disable_suggestions", "Disables suggestions output",
                          StenoEngine::DisableSuggestions_Binding, this);
  console.RegisterCommand("enable_text_log", "Enables text log output",
                          StenoEngine::EnableTextLog_Binding, this);
  console.RegisterCommand("disable_text_log", "Disables text log output",
                          StenoEngine::DisableTextLog_Binding, this);
  console.RegisterCommand("lookup", "Looks up a word",
                          StenoEngine::Lookup_Binding, this);
  console.RegisterCommand("lookup_stroke", "Looks up a stroke",
                          StenoEngine::LookupStroke_Binding, this);
  console.RegisterCommand("remove_stroke",
                          "Removes a stroke from specified dictionary",
                          StenoEngine::RemoveStroke_Binding, this);
  console.RegisterCommand("process_strokes", "Processes a stroke list",
                          StenoEngine::ProcessStrokes_Binding, this);
  console.RegisterCommand("list_template_values", "Lists all template values",
                          StenoEngine::ListTemplateValues_Binding, this);
  console.RegisterCommand("set_template_value", "Sets template value",
                          StenoEngine::SetTemplateValue_Binding, this);
  console.RegisterCommand("enable_template_value_updates",
                          "Enables template value update events",
                          StenoEngine::EnableTemplateValueUpdate_Binding, this);
  console.RegisterCommand(
      "disable_template_value_updates", "Disables template value update events",
      StenoEngine::DisableTemplateValueUpdate_Binding, this);
}

//---------------------------------------------------------------------------
