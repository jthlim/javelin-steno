//---------------------------------------------------------------------------

#include "split.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

size_t TxBuffer::txPacketTypeCounts[(size_t)SplitHandlerId::COUNT];

//---------------------------------------------------------------------------

bool TxBuffer::Add(SplitHandlerId id, const void *data, size_t length) {
  uint32_t wordLength = (length + 3) >> 2;
  if (header.wordCount + 1 + wordLength > JAVELIN_SPLIT_TX_RX_BUFFER_SIZE) {
    return false;
  }

  txPacketTypeCounts[(size_t)id]++;

  uint32_t blockHeader = ((size_t)id << 16) | length;
  buffer[header.wordCount++] = blockHeader;

  memcpy(&buffer[header.wordCount], data, length);
  header.wordCount += wordLength;

  return true;
}

#endif

//---------------------------------------------------------------------------