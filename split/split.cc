//---------------------------------------------------------------------------

#include "split.h"
#include "../button_script_manager.h"
#include "../crc32.h"
#include "../hash.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

TxBuffer::Handlers TxBuffer::handlers;
size_t RxBuffer::rxPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
size_t TxBuffer::txPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
SplitRxHandler *RxBuffer::handlers[(size_t)SplitHandlerId::COUNT];

//---------------------------------------------------------------------------

inline uint32_t RotateLeft(uint32_t v, int shift) {
  return (v << shift) | (v >> (32 - shift));
}

uint16_t TxRxHeader::Hash(const uint32_t *data, size_t wordCount) {
#if JAVELIN_PLATFORM_PICO_SDK
  // Since the pico has a hardware accelerated Crc, just use that.
  return (uint16_t)Crc32::Hash(data, sizeof(uint32_t) * wordCount);
#else
  const uint32_t v = JavelinHash::Hash(data, wordCount);
  return (uint16_t)(v ^ (v >> 16));
#endif
}

//---------------------------------------------------------------------------

void TxBuffer::Build() {
  Reset();
  for (SplitTxHandler *handler : handlers) {
    handler->UpdateBuffer(*this);
  }
  UpdateHash();
}

void TxBuffer::BuildEmpty() {
  Reset();
#if JAVELIN_PLATFORM_PICO_SDK
  header.hash = Crc32::EmptyHash();
#else
  constexpr uint32_t v = JavelinHash::EmptyHash();
  constexpr uint16_t hash = (uint16_t)(v ^ (v >> 16));
  header.hash = hash;
#endif
}

void TxBuffer::UpdateHash() {
  header.hash = TxRxHeader::Hash(buffer, header.wordCount);
}

bool TxBuffer::Add(SplitHandlerId id, const void *data, size_t length) {
  void *target = Add(id, length);
  if (!target) {
    return false;
  }

  memcpy(target, data, length);

  return true;
}

// Reserves buffer space to write to.
// To commit the packet, a subsequent Add(id, length) is required immediately
// after.
uint8_t *TxBuffer::Reserve(size_t length) {
  const uint32_t wordLength = (length + 3) >> 2;
  if (header.wordCount + 1 + wordLength > JAVELIN_SPLIT_TX_RX_BUFFER_SIZE) {
    return nullptr;
  }
  return (uint8_t *)&buffer[header.wordCount + 1];
}

uint8_t *TxBuffer::Add(SplitHandlerId id, size_t length) {
  const uint32_t wordLength = (length + 3) >> 2;
  if (header.wordCount + 1 + wordLength > JAVELIN_SPLIT_TX_RX_BUFFER_SIZE) {
    return nullptr;
  }

  txPacketTypeCounts[(size_t)id]++;

  const uint32_t blockHeader = ((size_t)id << 16) | length;
  buffer[header.wordCount++] = blockHeader;

  uint8_t *result = (uint8_t *)&buffer[header.wordCount];
  header.wordCount += wordLength;

  return result;
}

void TxBuffer::Handlers::OnConnect() const {
  for (SplitTxHandler *handler : *this) {
    handler->OnTransmitConnected();
  }
  ButtonScriptManager::ExecuteScript(ButtonScriptId::PAIR_CONNECTION_UPDATE);
}

void TxBuffer::Handlers::OnConnectionReset() const {
  for (SplitTxHandler *handler : *this) {
    handler->OnTransmitConnectionReset();
  }
  ButtonScriptManager::ExecuteScript(ButtonScriptId::PAIR_CONNECTION_UPDATE);
}

//---------------------------------------------------------------------------

RxBufferValidateResult RxBuffer::Validate(size_t totalWordsReceived,
                                          size_t *metrics) const {
  if (totalWordsReceived < sizeof(TxRxHeader) / sizeof(uint32_t)) {
    // Header has not been received.
    metrics[SplitMetricId::WAITING_FOR_HEADER_COUNT]++;
    return RxBufferValidateResult::CONTINUE;
  }

  if (header.magic != TxRxHeader::MAGIC) {
    // Error: Magic mismatch!
    metrics[SplitMetricId::MAGIC_MISMATCH_COUNT]++;
    return RxBufferValidateResult::ERROR;
  }

  const size_t bufferCount =
      totalWordsReceived - sizeof(TxRxHeader) / sizeof(uint32_t);
  if (bufferCount < header.wordCount) {
    // Data has not been fully received.
    metrics[SplitMetricId::WAITING_FOR_DATA_COUNT]++;
    return RxBufferValidateResult::CONTINUE;
  }

  if (header.wordCount != bufferCount) {
    // Excess data has been received.
    metrics[SplitMetricId::EXCESS_DATA_COUNT]++;
  }

  const uint16_t expectedHash = TxRxHeader::Hash(buffer, header.wordCount);
  if (header.hash != expectedHash) {
    // Hash failure.
    metrics[SplitMetricId::HASH_FAILURE_COUNT]++;
    return RxBufferValidateResult::ERROR;
  }

  return RxBufferValidateResult::OK;
}

[[gnu::weak]] void RxBuffer::OnDataReceived() {}

void RxBuffer::Process() const {
  if (header.wordCount == 0) {
    return;
  }

  OnDataReceived();

  size_t offset = 0;
  while (offset < header.wordCount) {
    const uint32_t blockHeader = buffer[offset++];
    const uint32_t type = blockHeader >> 16;
    rxPacketTypeCounts[type]++;
    const size_t length = blockHeader & 0xffff;

    if (type < (size_t)SplitHandlerId::COUNT) {
      SplitRxHandler *handler = handlers[type];
      if (handler != nullptr) {
        handler->OnDataReceived(&buffer[offset], length);
      }
    }

    const uint32_t wordLength = (length + 3) >> 2;
    offset += wordLength;
  }
}

void RxBuffer::OnConnect() {
  for (SplitRxHandler *handler : handlers) {
    if (handler) {
      handler->OnReceiveConnected();
    }
  }
}

void RxBuffer::OnConnectionReset() {
  for (SplitRxHandler *handler : handlers) {
    if (handler) {
      handler->OnReceiveConnectionReset();
    }
  }
}

//---------------------------------------------------------------------------

[[gnu::weak]] bool Split::IsPairConnected() { return false; }

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------
