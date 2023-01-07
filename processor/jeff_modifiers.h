//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Implementation of: https://github.com/jthlim/jeff-modifiers in firmware.
//
// One key difference is that the modifier keys state is *held down*, so that
// actions such as alt-tab can be achieved (when using first-up triggering).
class StenoJeffModifiers final : public StenoProcessorElement {
public:
  StenoJeffModifiers(StenoProcessorElement &nextProcessor)
      : nextProcessor(nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void Tick() { nextProcessor.Tick(); }
  void PrintInfo() const final;

  static const StenoStroke TRIGGER_STROKE;
  static const StenoStroke TRIGGER_MASK;
  static const StenoStroke DATA_MASK;
  static const StenoStroke IGNORE_MASK;

  static const StenoStroke CONTROL_MASK;
  static const StenoStroke SHIFT_MASK;
  static const StenoStroke SUPER_MASK;
  static const StenoStroke ALT_MASK;

private:
  bool wasModifier = false;

  bool wasControl = false;
  bool wasShift = false;
  bool wasSuper = false;
  bool wasAlt = false;

  StenoStroke lastModifiers;
  StenoProcessorElement &nextProcessor;

  void UpdateModifiers(StenoStroke stroke);
  bool TriggerSendKey(StenoStroke stroke) const;
};

//---------------------------------------------------------------------------
