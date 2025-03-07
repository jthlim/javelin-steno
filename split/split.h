//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include "../container/fast_iterable.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum class SplitHandlerId {
  KEY_STATE,
  RGB,
  HID_REPORT,
  CONSOLE,
  SERIAL,
  HID_BUFFER_SIZE,
  DISPLAY_DATA,
  DISPLAY_AVAILABLE,
  DISPLAY_CONTROL,
  USB_STATUS,
  PAIR_CONSOLE,
  U2F,
  POWER_OVERRIDE,
  ENCODER,
  VERSION,
  POINTER,

  COUNT,
};

enum SplitMetricId {
  RESET_COUNT,
  TIMEOUT_COUNT,
  MAGIC_MISMATCH_COUNT,
  HASH_FAILURE_COUNT,
  WAITING_FOR_HEADER_COUNT,
  WAITING_FOR_DATA_COUNT,
  EXCESS_DATA_COUNT,
  REPEAT_DATA_COUNT,

  COUNT,
};

#if !defined(JAVELIN_SPLIT_TX_RX_BUFFER_SIZE)
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 2048
#endif

struct TxRxHeader {
  uint16_t magic;
  uint16_t wordCount;
  uint16_t transferId;
  uint16_t hash;

  static constexpr uint16_t MAGIC = 0x534a; // 'JS';

  size_t GetByteCount() const {
    return sizeof(*this) + wordCount * sizeof(uint32_t);
  }
  size_t GetWordCount() const {
    return sizeof(*this) / sizeof(uint32_t) + wordCount;
  }

  static uint16_t Hash(const uint32_t *data, size_t wordCount);
};

class SplitTxHandler;
class SplitRxHandler;

class TxBuffer {
public:
  TxBuffer() { header.magic = TxRxHeader::MAGIC; }

  void Reset() { header.wordCount = 0; }
  bool Add(SplitHandlerId id, const void *data, size_t length);
  uint8_t *Add(SplitHandlerId id, size_t length);
  uint8_t *Reserve(size_t length);
  void Build();
  void BuildEmpty();
  void UpdateHash();
  size_t GetByteCount() const { return header.GetByteCount(); }
  size_t GetWordCount() const { return header.GetWordCount(); }

  TxRxHeader header;
  uint32_t buffer[JAVELIN_SPLIT_TX_RX_BUFFER_SIZE];

  static void OnConnectionReset() { handlers.OnConnectionReset(); }
  static void OnConnect() { handlers.OnConnect(); }

  struct Handlers
      : public FastIterableStaticList<SplitTxHandler *,
                                      (size_t)SplitHandlerId::COUNT> {
    void OnConnect() const;
    void OnConnectionReset() const;
  };

  static Handlers handlers;
  static size_t txPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
};

enum class RxBufferValidateResult {
  OK,
  CONTINUE,
  ERROR,
};

struct RxBuffer {
  TxRxHeader header;
  uint32_t buffer[JAVELIN_SPLIT_TX_RX_BUFFER_SIZE];

  size_t GetByteCount() const { return header.GetByteCount(); }
  size_t GetWordCount() const { return header.GetWordCount(); }

  RxBufferValidateResult Validate(size_t totalWordsReceived,
                                  size_t *metrics) const;

  void Process() const;
  static void OnDataReceived();

  static void OnConnect();
  static void OnConnectionReset();

  static SplitRxHandler *handlers[];
  static size_t rxPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
};

class SplitTxHandler {
public:
  virtual void OnTransmitConnected() {}
  virtual void OnTransmitConnectionReset() {}
  virtual void UpdateBuffer(TxBuffer &buffer) = 0;
};

class SplitRxHandler {
public:
  virtual void OnReceiveConnected() {}
  virtual void OnReceiveConnectionReset() {}
  virtual void OnDataReceived(const void *data, size_t length) = 0;
};

class Split {
public:
#if JAVELIN_SPLIT
  static bool IsMaster() { return JAVELIN_SPLIT_IS_MASTER; }
  static bool IsPairConnected();

#if defined(JAVELIN_SPLIT_IS_LEFT)
  static bool IsLeft() { return JAVELIN_SPLIT_IS_LEFT; }
#else
  static bool IsLeft();
#endif

  static void RegisterTxHandler(SplitTxHandler *handler) {
    TxBuffer::handlers.Add(handler);
  }
  static void RegisterRxHandler(SplitHandlerId id, SplitRxHandler *handler) {
    RxBuffer::handlers[(size_t)id] = handler;
  }
#else
  static bool IsMaster() { return true; }
  static bool IsPairConnected() { return false; }

  static void RegisterTxHandler(void *handler) {}
  static void RegisterRxHandler(SplitHandlerId id, void *handler) {}
#endif

  static bool IsSlave() { return !IsMaster(); }
};

//---------------------------------------------------------------------------
