//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Implementation of: https://github.com/jthlim/jeff-modifiers in firmware.
//
// One key difference is that the modifier keys state is *held down*, so that
// actions such as alt-tab can be achieved (when using first-up triggering).
class JeffModifiers : public StenoProcessorElement {
public:
  JeffModifiers(StenoProcessorElement &nextProcessor)
      : nextProcessor(nextProcessor) {}

  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() { nextProcessor.Tick(); }

  static const StenoChord TRIGGER_CHORD;
  static const StenoChord TRIGGER_MASK;
  static const StenoChord DATA_MASK;
  static const StenoChord IGNORE_MASK;

  static const StenoChord CONTROL_MASK;
  static const StenoChord SHIFT_MASK;
  static const StenoChord SUPER_MASK;
  static const StenoChord ALT_MASK;

private:
  bool wasModifier = false;

  bool wasControl = false;
  bool wasShift = false;
  bool wasSuper = false;
  bool wasAlt = false;

  StenoChord lastModifiers;
  StenoProcessorElement &nextProcessor;

  void UpdateModifiers(StenoChord chord);
  bool TriggerSendKey(StenoChord chord) const;
};

//---------------------------------------------------------------------------
