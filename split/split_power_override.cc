//---------------------------------------------------------------------------

#include "split_power_override.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitPowerOverride SplitPowerOverride::instance;

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER

void SplitPowerOverride::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;

  buffer.Add(SplitHandlerId::POWER_OVERRIDE, &data, sizeof(data));
}

#else

bool SplitPowerOverride::IsPowerRequired(bool localStatus) {
  switch (instance.data.override) {
  case PowerOverride::OFF:
    return false;

  case PowerOverride::ON:
    return true;

  default:
    return localStatus;
  }
}

void SplitPowerOverride::OnDataReceived(const void *data, size_t length) {
  const Data &newData = *(const Data *)data;
  if (this->data == newData) {
    return;
  }

  this->data = newData;
  OnOverrideUpdated();
}

void SplitPowerOverride::OnReceiveConnectionReset() {
  if (data.override == PowerOverride::NONE) {
    return;
  }
  data.override = PowerOverride::NONE;
  OnOverrideUpdated();
}

[[gnu::weak]] void SplitPowerOverride::OnOverrideUpdated() const {}

#endif

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------