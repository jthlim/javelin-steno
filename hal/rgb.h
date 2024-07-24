//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class Rgb {
public:
  static void SetRgb(size_t id, int r, int g, int b);

  // h = hue,        0-65536 represents 0°  - 360°
  // s = saturation, 0-256   represents 0.0 - 1.0
  // v = value,      0-255   represents 0.0 - 1.0
  static void SetHsv(size_t id, int h, int s, int v);

  static size_t GetCount();

  static void SetRgb_Binding(void *context, const char *commandLine);

#if JAVELIN_RGB
  static void AddConsoleCommands(Console &console);
#else
  static void AddConsoleCommands(Console &console) {}
#endif
};

//---------------------------------------------------------------------------
