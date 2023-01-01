//---------------------------------------------------------------------------

#pragma once
#include "chord_history.h"
#include "orthography.h"
#include "processor/processor.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"

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

  void Process(const StenoKeyState &value, StenoAction action);
  void ProcessUndo();
  void ProcessChord(StenoChord chord);
  void Tick() {}

  void SendText(const uint8_t *p);
  void PrintInfo() const;
  void PrintDictionary() const;

  void ListDictionaries();
  bool EnableDictionary(const char *name);
  bool DisableDictionary(const char *name);
  bool ToggleDictionary(const char *name);
  void ReverseLookup(StenoReverseDictionaryLookup &result);

  bool IsPaperTapeEnabled() const { return paperTapeEnabled; }
  void EnablePaperTape() { paperTapeEnabled = true; }
  void DisablePaperTape() { paperTapeEnabled = false; }

  bool IsSuggestionsEnabled() const { return suggestionsEnabled; }
  void EnableSuggestions() { suggestionsEnabled = true; }
  void DisableSuggestions() { suggestionsEnabled = false; }

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
  static void Lookup_Binding(void *context, const char *commandLine);

private:
  static const StenoChord UNDO_CHORD;
  static const size_t SEGMENT_CONVERSION_LIMIT = 32;
  static const size_t PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT = 8;

  bool paperTapeEnabled = false;
  bool suggestionsEnabled = false;
  StenoEngineMode mode = StenoEngineMode::NORMAL;

  uint32_t strokeCount = 0;
  StenoDictionary &dictionary;
  const StenoCompiledOrthography orthography;
  StenoUserDictionary *userDictionary;

  StenoState state;
  StenoState addTranslationState;

  StenoKeyCodeEmitter emitter;

  ChordHistory history;
  ChordHistory addTranslationHistory;

  struct ConversionBuffer {
    ChordHistory chordHistory;
    StenoKeyCodeBuffer keyCodeBuffer;
  };

  ConversionBuffer previousConversionBuffer;
  ConversionBuffer nextConversionBuffer;

  struct UpdateNormalModeTextBufferThreadData;

  void ProcessNormalModeUndo();
  void ProcessNormalModeChord(StenoChord chord);
  void InitiateAddTranslationMode();
  void ProcessAddTranslationModeUndo();
  void ProcessAddTranslationModeChord(StenoChord chord);
  bool IsNewline(StenoChord chord) const;
  void EndAddTranslationMode();
  void AddTranslation(size_t newlineIndex);
  void DeleteTranslation(size_t newlineIndex);
  void ResetState();

  // Returns the number of segments
  void UpdateNormalModeTextBuffer(size_t sourceChordCount,
                                  ConversionBuffer &buffer,
                                  size_t conversionLimit,
                                  StenoSegmentList &segmentList);

  void PrintPaperTape(StenoChord chord,
                      const StenoSegmentList &previousSegmentList,
                      const StenoSegmentList &nextSegmentList);
  void PrintPaperTapeUndo(size_t undoCount);
  void PrintSuggestions(const StenoSegmentList &previousSegmentList,
                        const StenoSegmentList &nextSegmentList);
  void PrintSuggestion(const char *p, size_t arrowPrefixCount, char *buffer,
                       size_t strokeThreshold);
  char *PrintSegmentSuggestion(size_t wordCount,
                               const StenoSegmentList &segmentList,
                               char *buffer, char *lastLookup);

  size_t UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer);

  friend class StenoEngineTester;
};

//---------------------------------------------------------------------------
