//---------------------------------------------------------------------------

#include "bootrom.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Bootrom::Launch() {}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

Bootrom Bootrom::instance;

void Bootrom::UpdateBuffer(TxBuffer &buffer) {
  if (!launchSlave) {
    return;
  }
  launchSlave = false;
  buffer.Add(SplitHandlerId::BOOTROM, nullptr, 0);
}

void Bootrom::OnDataReceived(const void *data, size_t length) { Launch(); }

#endif

//---------------------------------------------------------------------------
