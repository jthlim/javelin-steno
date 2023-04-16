//---------------------------------------------------------------------------

#pragma once
#include "hal/split.h"

//---------------------------------------------------------------------------

struct KeyboardLedStatusValue {
  union {
    struct {
      uint8_t numLock : 1;
      uint8_t capsLock : 1;
      uint8_t scrollLock : 1;
      uint8_t compose : 1;
      uint8_t kana : 1;
    };
    uint8_t value;
  };
};

static_assert(sizeof(KeyboardLedStatusValue) == 1,
              "Unexpected KeyboardLedStatusValue size");

#if JAVELIN_SPLIT

class KeyboardLedStatus {
public:
  static void Set(KeyboardLedStatusValue value) { instance.Set(value); }

  static void RegisterTxHandler() { Split::RegisterTxHandler(&instance); }

  static bool IsNumLockOn() { return instance.value.numLock; }
  static bool IsCapsLockOn() { return instance.value.capsLock; }
  static bool IsScrollLockOn() { return instance.value.scrollLock; }
  static bool IsComposeOn() { return instance.value.compose; }
  static bool IsKanaOn() { return instance.value.kana; }

  static bool GetLedStatus(int index) {
    return ((instance.value.value >> index) & 1) != 0;
  }

  static void RegisterRxHandler() {
    Split::RegisterRxHandler(SplitHandlerId::KEYBOARD_LED_STATUS, &instance);
  }

private:
  struct KeyboardLedStatusData : public SplitTxHandler, public SplitRxHandler {
    bool dirty = false;
    KeyboardLedStatusValue value;

    void Set(KeyboardLedStatusValue newValue) {
      dirty = true;
      value = newValue;
    }

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnTransmitConnectionReset() { dirty = true; }
    virtual void OnDataReceived(const void *data, size_t length);
  };

  static KeyboardLedStatusData instance;
};

#else

class KeyboardLedStatus {
public:
  static void Set(KeyboardLedStatusValue value) { instance = value; }

  static bool IsNumLockOn() { return instance.numLock; }
  static bool IsCapsLockOn() { return instance.capsLock; }
  static bool IsScrollLockOn() { return instance.scrollLock; }
  static bool IsComposeOn() { return instance.compose; }
  static bool IsKanaOn() { return instance.kana; }

  static bool GetLedStatus(int index) {
    return ((instance.value >> index) & 1) != 0;
  }

  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}

private:
  static KeyboardLedStatusValue instance;
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
