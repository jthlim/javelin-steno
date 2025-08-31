//---------------------------------------------------------------------------

#include "serial_port.h"

#if RUN_TESTS

std::vector<uint8_t> SerialPort::history;

#endif

[[gnu::weak]] void SerialPort::SendData(const void *data, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    SendByte(((const uint8_t*)data)[i]);
  }
}

//---------------------------------------------------------------------------
