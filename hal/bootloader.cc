//---------------------------------------------------------------------------

#include "bootloader.h"
#include "../console.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Bootloader::Launch() {}

//---------------------------------------------------------------------------

void Bootloader::AddConsoleCommands(Console &console) {
  console.RegisterCommand("launch_bootloader", "Launch bootloader",
                          &LaunchBootloader, nullptr);
}

//---------------------------------------------------------------------------