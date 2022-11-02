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
class StenoUserDictionary;

struct StenoConfigBlock;

//---------------------------------------------------------------------------

enum StenoEngineMode { NORMAL, ADD_TRANSLATION };

//---------------------------------------------------------------------------

class StenoEngine final : public StenoProcessorElement {
public:
  StenoEngine(const StenoDictionary &dictionary,
              const StenoOrthography &orthography,
              StenoUserDictionary *userDictionary = nullptr);

  void Process(StenoKeyState value, StenoAction action) final;
  void ProcessUndo();
  void ProcessChord(StenoChord chord);
  void Tick() final {}

  void PrintInfo(const StenoConfigBlock *configBlock) const;
  void PrintDictionary() const;

private:
  static const StenoChord UNDO_CHORD;
  static const size_t SEGMENT_CONVERSION_LIMIT = 32;

  StenoEngineMode mode = StenoEngineMode::NORMAL;

  uint32_t strokeCount = 0;
  const StenoDictionary &dictionary;
  const StenoCompiledOrthography orthography;
  StenoUserDictionary *userDictionary;

  StenoState state = {
      .caseMode = StenoCaseMode::NORMAL,
      .joinNext = true,
      .isGlue = false,
      .spaceCharacterLength = 1,
      .spaceCharacter = " ",
  };
  StenoState addTranslationState;

  StenoKeyCodeEmitter emitter;

  ChordHistory history;
  ChordHistory addTranslationHistory;
  StenoKeyCodeBuffer previousKeyCodeBuffer;
  StenoKeyCodeBuffer nextKeyCodeBuffer;

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

  // Returns the number of segments
  size_t UpdateNormalModeTextBuffer(size_t maximumChordCount,
                                    StenoKeyCodeBuffer &buffer,
                                    size_t chordLength);

  size_t UpdateAddTranslationModeTextBuffer(StenoKeyCodeBuffer &buffer);

  friend class StenoEngineTester;
};

//---------------------------------------------------------------------------
