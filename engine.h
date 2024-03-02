//---------------------------------------------------------------------------

#pragma once
#include "orthography.h"
#include "processor/processor.h"
#include "segment_builder.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"
#include "stroke_history.h"

//---------------------------------------------------------------------------

class Pattern;
class StenoDictionary;
class StenoReverseDictionaryLookup;
class StenoSegmentList;
class StenoUserDictionary;

//---------------------------------------------------------------------------

enum StenoEngineMode { NORMAL, ADD_TRANSLATION };

//---------------------------------------------------------------------------

class StenoEngine final : public StenoProcessorElement {
public:
  StenoEngine(StenoDictionary &dictionary,
              const StenoCompiledOrthography &orthography,
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
  void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  bool IsPaperTapeEnabled() const { return paperTapeEnabled; }
  void EnablePaperTape() { paperTapeEnabled = true; }
  void DisablePaperTape() { paperTapeEnabled = false; }

  bool IsSuggestionsEnabled() const { return suggestionsEnabled; }
  void EnableSuggestions() { suggestionsEnabled = true; }
  void DisableSuggestions() { suggestionsEnabled = false; }

  bool IsTextLogEnabled() const { return textLogEnabled; }
  void EnableTextLog() { textLogEnabled = true; }
  void DisableTextLog() { textLogEnabled = false; }

  bool IsSpaceAfter() const { return placeSpaceAfter; }
  void SetSpaceAfter(bool spaceAfter) { placeSpaceAfter = spaceAfter; }

  static void SetSpacePosition_Binding(void *context, const char *commandLine);
  static void ListDictionaries_Binding(void *context, const char *commandLine);
  static void EnableDictionary_Binding(void *context, const char *commandLine);
  static void DisableDictionary_Binding(void *context, const char *commandLine);
  static void ToggleDictionary_Binding(void *context, const char *commandLine);
  static void PrintDictionary_Binding(void *context, const char *commandLine);
  static void EnablePaperTape_Binding(void *context, const char *commandLine);
  static void DisablePaperTape_Binding(void *context, const char *commandLine);
  static void EnableSuggestions_Binding(void *context, const char *commandLine);
  static void DisableSuggestions_Binding(void *context,
                                         const char *commandLine);
  static void EnableTextLog_Binding(void *context, const char *commandLine);
  static void DisableTextLog_Binding(void *context, const char *commandLine);
  static void Lookup_Binding(void *context, const char *commandLine);
  static void LookupStroke_Binding(void *context, const char *commandLine);
  static void ProcessStrokes_Binding(void *context, const char *commandLine);

private:
  static const StenoStroke UNDO_STROKE;
  static const size_t SEGMENT_CONVERSION_PREFIX_SUFFIX_LIMIT = 4;
  static const size_t PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT = 8;

  bool paperTapeEnabled = false;
  bool suggestionsEnabled = false;
  bool textLogEnabled = false;
  bool placeSpaceAfter = false;
  StenoEngineMode mode = StenoEngineMode::NORMAL;

  size_t strokeCount = 0;
  StenoDictionary &dictionary;
  const StenoCompiledOrthography orthography;
  StenoUserDictionary *userDictionary;

  StenoState state;
  StenoState addTranslationState;

  StenoKeyCodeEmitter emitter;

  StenoStrokeHistory history;
  StenoStrokeHistory addTranslationHistory;

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

  struct UpdateNormalModeTextBufferThreadData;

  void ProcessNormalModeUndo();
  void ProcessNormalModeStroke(StenoStroke stroke);
  void InitiateAddTranslationMode();
  void ProcessAddTranslationModeUndo();
  void ProcessAddTranslationModeStroke(StenoStroke stroke);
  bool HandleAddTranslationModeScanCode(uint32_t scanCodeAndModifiers,
                                        ScanCodeAction action);
  bool IsNewline(StenoStroke stroke) const;
  void EndAddTranslationMode();
  void AddTranslation(size_t newlineIndex);
  void DeleteTranslation(size_t newlineIndex);
  void ResetState();
  static void Pump();

  void CreateSegments(size_t sourceStrokeCount, ConversionBuffer &buffer,
                      size_t conversionLimit, StenoSegmentList &segmentList);
  void CreateSegmentsUsingLongerResult(
      size_t sourceStrokeCount, ConversionBuffer &buffer,
      size_t conversionLimit, StenoSegmentList &segmentList,
      const ConversionBuffer &longerBuffer,
      const StenoSegmentList &longerSegmentList);
  void ConvertText(ConversionBuffer &buffer, StenoSegmentList &segmentList,
                   size_t startingOffset);

  void PrintPaperTape(StenoStroke stroke,
                      const StenoSegmentList &previousSegmentList,
                      const StenoSegmentList &nextSegmentList) const;
  void PrintPaperTapeUndo(size_t undoCount) const;
  void PrintSuggestions(const StenoSegmentList &previousSegmentList,
                        const StenoSegmentList &nextSegmentList);
  void PrintSuggestion(const char *p, size_t arrowPrefixCount,
                       size_t strokeThreshold) const;
  char *PrintSegmentSuggestion(size_t wordCount,
                               const StenoSegmentList &segmentList,
                               char *lastLookup);
  void PrintTextLog(const StenoKeyCodeBuffer &previousKeyCodeBuffer,
                    const StenoKeyCodeBuffer &nextKeyCodeBuffer) const;

  size_t UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer);

  friend class StenoEngineTester;
};

//---------------------------------------------------------------------------
