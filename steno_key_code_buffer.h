//---------------------------------------------------------------------------

#pragma once

#include "container/list.h"
#include "dictionary/dictionary.h"
#include "orthography.h"
#include "segment.h"
#include "state.h"
#include "steno_key_code.h"
#include <stddef.h>

//---------------------------------------------------------------------------

class StenoCompiledOrthography;

//---------------------------------------------------------------------------

// Large statically allocated buffers to avoid fragmentation preventing them
// from being allocated.
//
// StenoTokens are converted directly into these buffers, and functions are
// applied directly on them.
class StenoKeyCodeBuffer {
public:
  void Prepare(const StenoCompiledOrthography *newOrthography,
               StenoDictionary *newRootDictionary) {
    orthography = newOrthography;
    rootDictionary = newRootDictionary;
  }

  void Populate(StenoTokenizer &tokenizer, bool executeSideEffects);
  void Append(StenoTokenizer &tokenizer, bool executeSideEffects);

  static constexpr size_t BUFFER_SIZE = 2048;

  const StenoCompiledOrthography *orthography;
  StenoDictionary *rootDictionary;

  bool executeSideEffects = false;
  bool wasLastActionAStitch = false;
  bool launchConsole = false;
  bool launchAddTranslation = false;
  bool doResetState = false;
  StenoKeyCode *lastText;
  StenoState state;
  char *addTranslationText = nullptr;
  StenoKeyCode *currentOutput;
  StenoKeyCode buffer[BUFFER_SIZE];

  void Reset();
  size_t GetCount() const { return currentOutput - buffer; }

  StenoState GetPersistentState() const { return state.GetPersistentState(); }

  void ProcessText(const char *text, size_t length);
  void ProcessCommand(const char *command, size_t length);
  void ProcessOrthographicSuffix(const char *text, size_t length);

  void AppendSpace();
  void AppendText(const char *p, size_t n, StenoCaseMode outputCaseMode);
  void AppendTextNoCaseModeOverride(const char *p, size_t n,
                                    StenoCaseMode outputCaseMode);

  char *ToString(size_t startingOffset = 0) const;
  char *ToUnresolvedString() const;

  static bool IsGlue(const char *p);

  bool ProcessKeyPresses(const char *p, const char *end);
  void ReleaseKeyStack(List<KeyCode> &keyPressStack);

  static const char *AddParameter(List<char *> &parameters, const char *p,
                                  const char *end);

  void Backspace(int count);
  void RetroactiveCapitalize(int count);
  void RetroactiveUncapitalize(int count);
  void RetroactiveTitleCase(int count);
  void RetroactiveUpperCase(int count);
  void RetroactiveLowerCase(int count);
  void RetroactiveReplaceSpace(int count, const char *replacement);
  void RetroactiveQuotes(int count, const char *startQuote,
                         const char *endQuote);
  void RetroactiveSingleQuotes(int count);
  void RetroactiveDoubleQuotes(int count);
  void RetroactiveDeleteSpace();
  void RetroactiveFormatCurrency(const char *pStart, const char *pEnd);
  void RepeatLastCharacterCount(int count);
  void RepeatLastFragmentCount(int count);
  void RepeatLastWordCount(int count);

  bool ProcessFunction(const List<char *> &parameters);

  // parameters[0] == function name.
  bool AddTranslationFunction(const List<char *> &parameters);
  bool ConsoleFunction(const List<char *> &parameters);
  bool DisableDictionaryFunction(const List<char *> &parameters);
  bool EnableDictionaryFunction(const List<char *> &parameters);
  bool HostLayoutFunction(const List<char *> &parameters);
  bool RepeatLastCharacter(const List<char *> &parameters);
  bool RepeatLastFragment(const List<char *> &parameters);
  bool RepeatLastWord(const List<char *> &parameters);
  bool ResetStateFunction(const List<char *> &parameters);
  bool RetroCapitalizeFunction(const List<char *> &parameters);
  bool RetroDoubleQuotesFunction(const List<char *> &parameters);
  bool RetroLowerCaseFunction(const List<char *> &parameters);
  bool RetroReplaceSpaceFunction(const List<char *> &parameters);
  bool RetroSingleQuotesFunction(const List<char *> &parameters);
  bool RetroSurroundFunction(const List<char *> &parameters);
  bool RetroTitleCaseFunction(const List<char *> &parameters);
  bool RetroUpperCaseFunction(const List<char *> &parameters);
  bool SetCaseFunction(const List<char *> &parameters);
  bool SetSpaceFunction(const List<char *> &parameters);
  bool StitchFunction(const List<char *> &parameters);
  bool StitchLastWordFunction(const List<char *> &parameters);
  bool ToggleDictionaryFunction(const List<char *> &parameters);
  bool UnicodeFunction(const List<char *> &parameters);

  void operator=(const StenoKeyCodeBuffer &o);

private:
  static void Reverse(StenoKeyCode *start, StenoKeyCode *end);

  bool CountHandler(void (StenoKeyCodeBuffer::*handler)(int),
                    const List<char *> &parameters);
};

//---------------------------------------------------------------------------
