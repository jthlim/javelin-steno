//---------------------------------------------------------------------------

#include "split_version.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

#include "../dictionary/invalid_dictionary.h"
#include "../engine.h"

//---------------------------------------------------------------------------

SplitVersion SplitVersion::instance;

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER
void SplitVersion::OnReceiveConnectionReset() { ClearError(); }

void SplitVersion::OnDataReceived(const void *data, size_t length) {
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
  if (!isDirty) [[likely]] {
    return;
  }

  constexpr int version = VERSION;
  if (buffer.Add(SplitHandlerId::VERSION, &version, sizeof(version))) {
    isDirty = false;
  }
}

#endif

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
