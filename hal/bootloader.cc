//---------------------------------------------------------------------------

#include "bootloader.h"
#include "../console.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Bootloader::Launch() {}

//---------------------------------------------------------------------------

void Bootloader::AddConsoleCommands(Console &console) {
  console.RegisterCommand("launch_bootloader", "Launch bootloader",
                          &LaunchBootloader, nullptr);
}

//---------------------------------------------------------------------------