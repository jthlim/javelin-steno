//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class SerialPort {
public:
#if RUN_TESTS
  static void SendByte(const uint8_t byte) { history.push_back(byte); }

  static std::vector<uint8_t> history;
#else
  static void SendByte(const uint8_t byte);
#endif

  static void SendData(const uint8_t *data, size_t length);
};

//---------------------------------------------------------------------------
