//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum class SplitHandlerId {
  KEY_STATE,
  RGB,
  HID_REPORT,
  CONSOLE,
  BOOTROM,
  KEYBOARD_LED_STATUS,
  SERIAL,
  HID_BUFFER_SIZE,
  OLED_DATA,
  OLED_AVAILABLE,
  OLED_CONTROL,
  USB_STATUS,

  COUNT,
};

#if !defined(JAVELIN_SPLIT_TX_RX_BUFFER_SIZE)
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 2048
#endif

struct TxRxHeader {
  uint16_t magic;
  uint16_t wordCount;
  uint32_t crc;

  static const uint16_t MAGIC = 0x534a; // 'JS';
};

class TxBuffer {
public:
  TxBuffer() { header.magic = TxRxHeader::MAGIC; }

  void Reset() { header.wordCount = 0; }
  bool Add(SplitHandlerId id, const void *data, size_t length);

  TxRxHeader header;
  uint32_t buffer[JAVELIN_SPLIT_TX_RX_BUFFER_SIZE];

  static size_t txPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
};

struct RxBuffer {
  TxRxHeader header;
  uint32_t buffer[JAVELIN_SPLIT_TX_RX_BUFFER_SIZE];
};

class SplitTxHandler {
public:
  virtual void OnTransmitConnectionReset() {}
  virtual void OnTransmitSucceeded() {}
  virtual void UpdateBuffer(TxBuffer &buffer) = 0;
};

class SplitRxHandler {
public:
  virtual void OnReceiveConnectionReset() {}
  virtual void OnDataReceived(const void *data, size_t length) = 0;
};

class Split {
public:
#if JAVELIN_SPLIT
  static bool IsMaster() { return JAVELIN_SPLIT_IS_MASTER; }

#if defined(JAVELIN_SPLIT_IS_LEFT)
  static bool IsLeft() { return JAVELIN_SPLIT_IS_LEFT; }
#else
  static bool IsLeft();
#endif

  static void RegisterTxHandler(SplitTxHandler *handler);
  static void RegisterRxHandler(SplitHandlerId id, SplitRxHandler *handler);
#else
  static bool IsMaster() { return true; }

  static void RegisterTxHandler(void *handler);
  static void RegisterRxHandler(SplitHandlerId id, void *handler);
#endif

  static bool IsSlave() { return !IsMaster(); }
};

//---------------------------------------------------------------------------
