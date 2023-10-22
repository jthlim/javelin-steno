//---------------------------------------------------------------------------

#pragma once
#include "../split/split.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class Bootloader {
public:
  static void Launch();
  static void LaunchBootloader(void *const, const char *commandLine) {
    Launch();
  }
  static void AddConsoleCommands(Console &console);
};

//---------------------------------------------------------------------------
