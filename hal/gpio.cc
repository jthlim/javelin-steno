//---------------------------------------------------------------------------

#include "gpio.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Gpio::SetInputPin(int pin, Pull pull) {}
__attribute__((weak)) bool Gpio::GetPin(int pin) {}
__attribute__((weak)) void Gpio::SetPin(int pin, bool value) {}

//---------------------------------------------------------------------------
