//---------------------------------------------------------------------------

#include "../timer_manager.h"
#include "passthrough.h"

//---------------------------------------------------------------------------

class StenoRepeat final : public StenoPassthrough, public TimerHandler {
private:
  using super = StenoPassthrough;

public:
  StenoRepeat(StenoProcessorElement &nextProcessor) : super(&nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void Tick();
  void PrintInfo() const;

  static const int FORGET_TIME_SPAN = 120;
  static const int INITIAL_REPEAT_DELAY = 200;
  static const int REPEAT_DELAY = 30;
  static const int REPEAT_DELAY_MINIMUM = 5;

private:
  bool wasLastEventAPress = false;

  bool isRepeating = false;

  uint32_t pressTime = 0;
  uint32_t releaseTime = 0;

  virtual void Run(intptr_t id);

  StenoKeyState pressedKeyState;
  StenoKeyState releasedKeyState;
};

//---------------------------------------------------------------------------
