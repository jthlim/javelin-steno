//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoRepeat final : public StenoProcessorElement {
public:
  StenoRepeat(StenoProcessorElement &nextProcessor)
      : nextProcessor(nextProcessor) {}

  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() final;

  static const int FORGET_TIME_SPAN = 120;
  static const int INITIAL_REPEAT_DELAY = 200;
  static const int REPEAT_DELAY = 30;

private:
  bool wasLastEventAPress = false;

  bool isRepeating = false;
  uint32_t nextTriggerTime = 0;

  uint32_t pressTime = 0;
  uint32_t releaseTime = 0;

  StenoKeyState pressedKeyState;
  StenoKeyState releasedKeyState;

  StenoProcessorElement &nextProcessor;
};

//---------------------------------------------------------------------------
