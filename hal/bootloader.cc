//---------------------------------------------------------------------------

#include "bootloader.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Bootloader::Launch() {}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

Bootloader Bootloader::instance;

void Bootloader::UpdateBuffer(TxBuffer &buffer) {
  if (!launchSlave) {
    return;
  }
  launchSlave = false;
  buffer.Add(SplitHandlerId::BOOTLOADER, nullptr, 0);
}

void Bootloader::OnDataReceived(const void *data, size_t length) { Launch(); }

#endif

//---------------------------------------------------------------------------
