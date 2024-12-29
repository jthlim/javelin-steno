//---------------------------------------------------------------------------

#include "gpio.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Gpio::SetInputPin(int pin, Pull pull) {}
[[gnu::weak]] bool Gpio::GetPin(int pin) { return false; }
[[gnu::weak]] void Gpio::SetPin(int pin, bool value) {}
[[gnu::weak]] void Gpio::SetPinDutyCycle(int pin, int dutyCycle) {}

//---------------------------------------------------------------------------
