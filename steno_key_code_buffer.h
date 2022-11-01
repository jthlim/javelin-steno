//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

#include "list.h"
#include "segment.h"
#include "state.h"
#include "steno_key_code.h"

//---------------------------------------------------------------------------

class StenoEngine;

//---------------------------------------------------------------------------

// Large statically allocated buffers to avoid fragmentation preventing them
// from being allocated.
//
// StenoTokens are converted directly into these buffers, and functions are
// applied directly on them.
class StenoKeyCodeBuffer {
public:
  void Populate(StenoTokenizer *tokenizer, StenoEngine &engine);
  void Append(StenoTokenizer *tokenizer, StenoEngine &engine);

  static const size_t BUFFER_SIZE = 8192;

  size_t count = 0;
  size_t addTranslationCount = 0;
  StenoState state;
  StenoKeyCode buffer[BUFFER_SIZE];

  void Reset();

  void ProcessText(const char *text);
  void ProcessCommand(const char *command, StenoEngine &engine);
  void ProcessOrthographicSuffix(const char *text, size_t length,
                                 StenoEngine &engine);

  void AppendText(const char *p, size_t n, StenoCaseMode caseMode);

  // For Debugging
  char *ToString();

  static bool IsGlue(const char *p);

  bool ProcessKeyPresses(const char *p, const char *end);

  void RetroactiveCapitalize(int count);
  void RetroactiveTitleCase(int count);
  void RetroactiveUpperCase(int count);
  void RetroactiveLowerCase(int count);
  void RetroactiveQuotes(int count, const char *startQuote,
                         const char *endQuote);
  void RetroactiveDeleteSpace();

  // parameters[0] == function name.
  bool ProcessFunction(const List<char *> &parameters);
  bool RetroCapitalizeFunction(const List<char *> &parameters);
  bool RetroTitleCaseFunction(const List<char *> &parameters);
  bool RetroUpperCaseFunction(const List<char *> &parameters);
  bool RetroLowerCaseFunction(const List<char *> &parameters);
  bool RetroSingleQuotesFunction(const List<char *> &parameters);
  bool RetroDoubleQuotesFunction(const List<char *> &parameters);
  bool UnicodeFunction(const List<char *> &parameters);

  void operator=(const StenoKeyCodeBuffer &o);

private:
  static void Reverse(StenoKeyCode *start, StenoKeyCode *end);
};

//---------------------------------------------------------------------------
