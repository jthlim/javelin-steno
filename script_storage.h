//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

// Structure to manage script storage on the device.
// Almost all of the logic for this data is managed in the web tools.
struct ScriptStorageData {
  uint32_t expectedScriptCrc;
  size_t length;
  uint8_t data[0];

  size_t GetEffectiveLength() const;
  static void HandleGetScriptStorageParameter();
};

//---------------------------------------------------------------------------
