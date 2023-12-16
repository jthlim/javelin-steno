//---------------------------------------------------------------------------

#include "split.h"
#include "../clock.h"
#include "../crc.h"
#include "../script_manager.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

TxBuffer::Handlers TxBuffer::handlers;
size_t RxBuffer::rxPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
size_t TxBuffer::txPacketTypeCounts[(size_t)SplitHandlerId::COUNT];
SplitRxHandler *RxBuffer::handlers[(size_t)SplitHandlerId::COUNT];

//---------------------------------------------------------------------------

void TxBuffer::Build() {
  Reset();
  for (size_t i = 0; i < handlers.handlerCount; ++i) {
    handlers.handlers[i]->UpdateBuffer(*this);
  }
  UpdateCrc();
}

void TxBuffer::BuildEmpty() {
  Reset();
  header.crc16 = 0; // This is the same as calling UpdateCrc();
}

void TxBuffer::UpdateCrc() {
  header.crc16 = (uint16_t)Crc32(buffer, sizeof(uint32_t) * header.wordCount);
}

bool TxBuffer::Add(SplitHandlerId id, const void *data, size_t length) {
  void *target = Add(id, length);
  if (!target) {
    return false;
  }

  memcpy(target, data, length);

  return true;
}

uint8_t *TxBuffer::Add(SplitHandlerId id, size_t length) {
  uint32_t wordLength = (length + 3) >> 2;
  if (header.wordCount + 1 + wordLength > JAVELIN_SPLIT_TX_RX_BUFFER_SIZE) {
    return nullptr;
  }

  txPacketTypeCounts[(size_t)id]++;

  uint32_t blockHeader = ((size_t)id << 16) | length;
  buffer[header.wordCount++] = blockHeader;

  uint8_t *result = (uint8_t *)&buffer[header.wordCount];
  header.wordCount += wordLength;

  return result;
}

void TxBuffer::Handlers::OnConnectionReset() const {
  for (size_t i = 0; i < handlerCount; ++i) {
    handlers[i]->OnTransmitConnectionReset();
  }
  ScriptManager::ExecuteScript(ScriptId::PAIR_CONNECTION_UPDATE);
}

void TxBuffer::Handlers::OnConnect() const {
  for (size_t i = 0; i < handlerCount; ++i) {
    handlers[i]->OnTransmitConnected();
  }
  ScriptManager::ExecuteScript(ScriptId::PAIR_CONNECTION_UPDATE);
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

  size_t bufferCount =
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

  uint16_t expectedCrc =
      (uint16_t)Crc32(buffer, sizeof(uint32_t) * header.wordCount);
  if (header.crc16 != expectedCrc) {
    // Crc failure.
    metrics[SplitMetricId::CRC_FAILURE_COUNT]++;
    return RxBufferValidateResult::ERROR;
  }

  return RxBufferValidateResult::OK;
}

__attribute__((weak)) void RxBuffer::OnDataReceived() {}

void RxBuffer::Process() const {
  if (header.wordCount == 0) {
    return;
  }

  OnDataReceived();

  size_t offset = 0;
  while (offset < header.wordCount) {
    uint32_t blockHeader = buffer[offset++];
    uint32_t type = blockHeader >> 16;
    rxPacketTypeCounts[type]++;
    size_t length = blockHeader & 0xffff;

    if (type < (size_t)SplitHandlerId::COUNT) {
      SplitRxHandler *handler = handlers[type];
      if (handler != nullptr) {
        handler->OnDataReceived(&buffer[offset], length);
      }
    }

    uint32_t wordLength = (length + 3) >> 2;
    offset += wordLength;
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

__attribute__((weak)) bool Split::IsPairConnected() { return false; }

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------
