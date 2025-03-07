//---------------------------------------------------------------------------

#include "gpio.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Gpio::SetInputPin(int pin, Pull pull) {}
[[gnu::weak]] bool Gpio::GetPin(int pin) { return false; }
[[gnu::weak]] void Gpio::SetPin(int pin, bool value) {}
[[gnu::weak]] void Gpio::SetPinDutyCycle(int pin, int dutyCycle) {}

//---------------------------------------------------------------------------

void Gpio::AddConsoleCommands(Console &console) {
  console.RegisterCommand(
      "set_gpio_pin",
      "Set GPIO pin, e.g. \"set_gpio_pin 24 1\" or \"set_gpio_pin 24 30%\"",
      &SetPin_Binding, nullptr);
}

void Gpio::SetPin_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }

  int pin;
  p = Str::ParseInteger(&pin, p + 1, false);
  if (!p) {
    Console::Printf("ERR Missing pin parameter\n\n");
    return;
  }

  int value;
  p = Str::ParseInteger(&value, p + 1, false);
  if (!p) {
    Console::Printf("ERR Missing value parameter\n\n");
    return;
  }

  if (*p == '%') {
    SetPinDutyCycle(pin, value);
  } else {
    SetPin(pin, value != 0);
  }

  Console::SendOk();
}

//---------------------------------------------------------------------------
