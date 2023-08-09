//---------------------------------------------------------------------------

#pragma once
#include "../split/split.h"

//---------------------------------------------------------------------------

class Bootloader {
public:
  static void Launch();
  static void LaunchBootloader(void *const, const char *commandLine) {
    Launch();
  }
};

//---------------------------------------------------------------------------
