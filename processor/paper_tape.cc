//---------------------------------------------------------------------------

#include "paper_tape.h"
#include "../console.h"

//---------------------------------------------------------------------------

bool PaperTape::isPaperTapeEnabled = false;

//---------------------------------------------------------------------------

void PaperTape::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER)
    return;
  if (!IsPaperTapeEnabled())
    return;
  const StenoStroke stroke = value.ToStroke();
  Console::Printf("EV {\"event\":\"paper_tape\",\"data\":\"%t\"}\n\n", &stroke);
}

void PaperTape::PrintInfo() const {}

void PaperTape::AddConsoleCommands(Console &console) {
  console.RegisterCommand("enable_paper_tape", "Enables paper tape output",
                          &EnablePaperTape_Binding, nullptr);
  console.RegisterCommand("disable_paper_tape", "Disables paper tape output",
                          &DisablePaperTape_Binding, nullptr);
}

void PaperTape::EnablePaperTape_Binding(void *context,
                                        const char *commandLine) {
  EnablePaperTape();
  Console::SendOk();
}

void PaperTape::DisablePaperTape_Binding(void *context,
                                         const char *commandLine) {
  DisablePaperTape();
  Console::SendOk();
}

//---------------------------------------------------------------------------
