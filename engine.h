//---------------------------------------------------------------------------

#pragma once
#include "orthography.h"
#include "processor/processor.h"
#include "segment_builder.h"
#include "static_allocate.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"
#include "stroke_history.h"

//---------------------------------------------------------------------------

class Console;
class Pattern;
class StenoDictionary;
class StenoReverseDictionaryLookup;
class StenoSegmentList;
class StenoUserDictionary;

//---------------------------------------------------------------------------

enum StenoEngineMode { NORMAL, ADD_TRANSLATION, CONSOLE };

//---------------------------------------------------------------------------

class StenoEngine final : public StenoProcessorElement {
public:
  StenoEngine(StenoDictionary &dictionary,
              const StenoCompiledOrthography &orthography,
              StenoStroke undoStroke = StenoStroke(StrokeMask::STAR),
              StenoUserDictionary *userDictionary = nullptr);

  size_t GetStrokeCount() const { return strokeCount; }

  void Process(const StenoKeyState &value, StenoAction action);
  void ProcessUndo();
  void ProcessStroke(StenoStroke stroke);
  bool ProcessScanCode(uint32_t scanCodeAndModifiers, ScanCodeAction action);

  void SendText(const uint8_t *p);
  void PrintInfo() const;
  void PrintDictionary(const char *name) const;

  void ListDictionaries() const;
  bool EnableDictionary(const char *name);
  bool DisableDictionary(const char *name);
  bool ToggleDictionary(const char *name);
  void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  StenoDictionary &GetDictionary() const { return *activeDictionary; }
  const StenoCompiledOrthography &GetOrthography() const { return orthography; }

  void SetErrorDictionary(StenoDictionary *dictionary) {
    activeDictionary = dictionary;
  }
  void ClearErrorDictionary() { activeDictionary = storedDictionaries; }

  bool IsSpaceAfter() const { return placeSpaceAfter; }
  void SetSpaceAfter(bool spaceAfter) { placeSpaceAfter = spaceAfter; }

  bool IsJoinNext() const { return state.joinNext; }

  void AddConsoleCommands(Console &console);

  static JavelinStaticAllocate<StenoEngine> container;
  static StenoEngine &GetInstance() { return container.value; }

  const char *GetTemplateValue(size_t index) const {
    if (index >= TEMPLATE_VALUE_COUNT)
      return "";
    return templateValues[index].GetValue();
  }

  // data needs to be dynamically allocated, and StenoEngine will assume
  // responsibility for its lifecycle.
  void SetTemplateValue(size_t index, char *data);

  char *SwapTemplateValue(size_t index, char *data);

  char *ConvertText(StenoSegmentList &segments, size_t startingOffset);

private:
  static constexpr size_t PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT = 8;

  StenoStroke undoStroke;
  bool placeSpaceAfter = false;
  StenoEngineMode mode = StenoEngineMode::NORMAL;

  size_t strokeCount = 0;

  // When error messages need to be displayed, activeDictionary is redirected
  // to an InvalidDictionary.
  StenoDictionary *activeDictionary;
  StenoDictionary *storedDictionaries;

  const StenoCompiledOrthography &orthography;
  StenoUserDictionary *const userDictionary;

  StenoState state;
  StenoState altTranslationState;
  char *addTranslationText = nullptr;

  StenoKeyCodeEmitter emitter;

  StenoStrokeHistory history;
  StenoStrokeHistory altTranslationHistory;

  struct ConversionBuffer {
    StenoSegmentBuilder segmentBuilder;
    StenoKeyCodeBuffer keyCodeBuffer;

    void Prepare(const StenoCompiledOrthography *orthography,
                 StenoDictionary *dictionary) {
      keyCodeBuffer.Prepare(orthography, dictionary);
    }
  };

  ConversionBuffer previousConversionBuffer;
  ConversionBuffer nextConversionBuffer;

  static constexpr size_t TEMPLATE_VALUE_COUNT = 64;
  struct TemplateValue {
    char *value;

    const char *GetValue() const { return value ? value : ""; }

    void Set(char *newValue);
    bool Set(size_t updateId, char *newValue);
  };
  TemplateValue templateValues[TEMPLATE_VALUE_COUNT] = {};

  struct ConvertTextData;

  void ProcessNormalModeUndo();
  void ProcessNormalModeStroke(StenoStroke stroke);

  void InitiateAddTranslationMode(const char *text);
  void ProcessAddTranslationModeUndo();
  void ProcessAddTranslationModeStroke(StenoStroke stroke);
  bool HandleAddTranslationModeScanCode(uint32_t scanCodeAndModifiers,
                                        ScanCodeAction action);
  void UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer);
  void EndAddTranslationMode(bool hasAddedTranslation);
  void FreeAddTranslationText();

  void InitiateConsoleMode();
  void ProcessConsoleModeUndo();
  void ProcessConsoleModeStroke(StenoStroke stroke);
  bool HandleConsoleModeScanCode(uint32_t scanCodeAndModifiers,
                                 ScanCodeAction action);
  void UpdateConsoleModeTextBuffer(ConversionBuffer &buffer);
  void ConsoleModeExecute();
  void EndConsoleMode();

  bool IsNewline(StenoStroke stroke) const;
  void AddTranslation(size_t newlineIndex);
  void DeleteTranslation(size_t newlineIndex);
  void ResetState();
  static void Pump();

  void CreateSegments(StenoSegmentList &segments, StenoSegmentBuilder &builder,
                      const StenoStroke *strokes, size_t length);

  void CreateSegments(BuildSegmentContext &context, size_t sourceStrokeCount,
                      ConversionBuffer &buffer, size_t conversionLimit,
                      StenoSegmentList &segments);
  void CreateSegmentsUsingLongerResult(BuildSegmentContext &context,
                                       size_t sourceStrokeCount,
                                       ConversionBuffer &buffer,
                                       size_t conversionLimit,
                                       StenoSegmentList &segments,
                                       const ConversionBuffer &longerBuffer,
                                       const StenoSegmentList &longerSegments);
  void ConvertText(StenoKeyCodeBuffer &keyCodeBuffer,
                   StenoSegmentList &segments, size_t startingOffset);

  void PrintPaperTape(StenoStroke stroke,
                      const StenoSegmentList &previousSegments,
                      const StenoSegmentList &nextSegments) const;
  void PrintPaperTapeUndo(size_t undoCount) const;
  void PrintSuggestions(const StenoSegmentList &previousSegments,
                        const StenoSegmentList &nextSegments);
  void PrintFingerSpellingSuggestions(const StenoSegmentList &previousSegments,
                                      const StenoSegmentList &nextSegments);
  void PrintSuggestion(const char *p, size_t arrowPrefixCount,
                       size_t strokeThreshold) const;
  char *PrintSegmentSuggestion(size_t startSegmentIndex,
                               size_t strokeThresholdCount,
                               const StenoSegmentList &segments,
                               char *lastLookup);
  void PrintTextLog(const StenoKeyCodeBuffer &previousKeyCodeBus,
                    const StenoKeyCodeBuffer &nextKeyCodeBuffer) const;

  size_t GetStartingStrokeForNormalModeProcessing() const;
  size_t GetStartingStrokeForNormalModeUndoProcessing(size_t undoCount) const;

  static void SetSpacePosition_Binding(void *context, const char *commandLine);
  static void ListDictionaries_Binding(void *context, const char *commandLine);
  static void EnableDictionary_Binding(void *context, const char *commandLine);
  static void DisableDictionary_Binding(void *context, const char *commandLine);
  static void ToggleDictionary_Binding(void *context, const char *commandLine);
  static void PrintDictionary_Binding(void *context, const char *commandLine);
  static void Lookup_Binding(void *context, const char *commandLine);
  static void LookupStroke_Binding(void *context, const char *commandLine);
  static void RemoveStroke_Binding(void *context, const char *commandLine);
  static void ProcessStrokes_Binding(void *context, const char *commandLine);

  static void ListTemplateValues_Binding(void *context,
                                         const char *commandLine);
  static void SetTemplateValue_Binding(void *context, const char *commandLine);

  friend class StenoEngineTester;
};

//---------------------------------------------------------------------------
