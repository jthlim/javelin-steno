//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class Gpio {
public:
  enum Pull {
    NONE,
    DOWN,
    UP,
  };

  static void SetInputPin(int pin, Pull pull);
  static bool GetPin(int pin);
  static void SetPin(int pin, bool value);
  static void SetPinDutyCycle(int pin, int dutyCycle);

  static void AddConsoleCommands(Console &console);
  static void SetPin_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
