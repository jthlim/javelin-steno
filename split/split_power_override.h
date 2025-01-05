//---------------------------------------------------------------------------

#pragma once
#include "split.h"

//---------------------------------------------------------------------------

enum class PowerOverride : uint8_t { NONE, OFF, ON };

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class SplitPowerOverride :
#if JAVELIN_SPLIT_IS_MASTER
    public SplitTxHandler
#else
    public SplitRxHandler
#endif
{
public:
  static void Set(PowerOverride override) {
#if JAVELIN_SPLIT_IS_MASTER

    if (override == instance.data.override) {
      return;
    }
    instance.data.override = override;
    instance.dirty = true;
#endif
  }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::POWER_OVERRIDE, &instance);
#endif
  }

#if JAVELIN_SPLIT_IS_MASTER
  static bool IsPowerRequired(bool localStatus) { return localStatus; }
#else
  static bool IsPowerRequired(bool localStatus);
#endif

private:
  bool dirty;
  struct Data {
    PowerOverride override;
    bool operator==(const Data &other) const = default;
  };
  Data data;

#if JAVELIN_SPLIT_IS_MASTER
  void OnTransmitConnected() final { dirty = true; }
  void UpdateBuffer(TxBuffer &buffer) final;
#else
  void OnReceiveConnectionReset() final;
  void OnDataReceived(const void *data, size_t length) final;

  void OnOverrideUpdated() const;
#endif

  static SplitPowerOverride instance;
};

#else

class SplitPowerOverride {
public:
  static void Set(PowerOverride override) {}
  static bool IsPowerRequired(bool localStatus) { return localStatus; }
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif

//---------------------------------------------------------------------------
