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
