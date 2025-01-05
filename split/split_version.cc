//---------------------------------------------------------------------------

#include "split_version.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

#include "../clock.h"
#include "../dictionary/invalid_dictionary.h"
#include "../engine.h"

//---------------------------------------------------------------------------

SplitVersion SplitVersion::instance;

// TODO: January 2026 -- Deprecate timer logic.
const int32_t VERSION_TIMER_ID =
    -('S' * 0x1000000 + 'V' * 0x10000 + 'E' * 0x100 + 'R');

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER
void SplitVersion::OnReceiveConnected() {
  // Set timer to set error message if no version is received within 1 second.
  TimerManager::instance.StartTimer(VERSION_TIMER_ID, 1000, false, this,
                                    Clock::GetMilliseconds());
}

void SplitVersion::Run(intptr_t id) { ShowError(); }

void SplitVersion::OnReceiveConnectionReset() {
  TimerManager::instance.StopTimer(VERSION_TIMER_ID, Clock::GetMilliseconds());
  ClearError();
}

void SplitVersion::OnDataReceived(const void *data, size_t length) {
  TimerManager::instance.StopTimer(VERSION_TIMER_ID, Clock::GetMilliseconds());
  const uint32_t *version = (const uint32_t *)data;
  if (*version != VERSION) {
    ShowError();
  } else {
    ClearError();
  }
}

void SplitVersion::ShowError() {
#if JAVELIN_USE_EMBEDDED_STENO
  StenoEngine::GetInstance().SetErrorDictionary(
      &StenoInvalidDictionary::invalidPairVersionInstance);
#endif
}

void SplitVersion::ClearError() {
#if JAVELIN_USE_EMBEDDED_STENO
  StenoEngine::GetInstance().ClearErrorDictionary();
#endif
}

#else
void SplitVersion::UpdateBuffer(TxBuffer &buffer) {
  if (!isDirty) {
    return;
  }

  const int version = VERSION;
  if (buffer.Add(SplitHandlerId::VERSION, &version, sizeof(version))) {
    isDirty = false;
  }
}

#endif

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
