//---------------------------------------------------------------------------

#pragma once
#include "processor.h"

//---------------------------------------------------------------------------

class Console;
class StenoKeyState;

//---------------------------------------------------------------------------

class PaperTape final : public StenoProcessorElement {
public:
  virtual void Process(const StenoKeyState &value, StenoAction action);
  virtual void PrintInfo() const;

  static bool IsPaperTapeEnabled() { return isPaperTapeEnabled; }
  static void EnablePaperTape() { isPaperTapeEnabled = true; }
  static void DisablePaperTape() { isPaperTapeEnabled = false; }

  static void AddConsoleCommands(Console &console);

private:
  static bool isPaperTapeEnabled;

  static void EnablePaperTape_Binding(void *context, const char *commandLine);
  static void DisablePaperTape_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
