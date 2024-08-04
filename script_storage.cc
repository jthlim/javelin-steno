//---------------------------------------------------------------------------

#include "script_storage.h"
#include "console.h"
#include "hal/external_flash.h"
#include "script_manager.h"

#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

void ScriptStorageData::HandleGetScriptStorageParameter() {
#if JAVELIN_USE_SCRIPT_STORAGE
  const ExternalFlashSentry sentry;
  const uint32_t scriptCrc = ScriptManager::GetInstance().Crc();
  if (scriptCrc != SCRIPT_STORAGE_ADDRESS->expectedScriptCrc) {
    Console::Printf("ERR No matching layout\n\n");
    return;
  }

  Console::Printf("\"%D\"\n\n", SCRIPT_STORAGE_ADDRESS->data,
                  SCRIPT_STORAGE_ADDRESS->length);
#else
  Console::Printf("ERR No layout storage\n\n");
#endif
}

//---------------------------------------------------------------------------