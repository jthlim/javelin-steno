//---------------------------------------------------------------------------

#include "split_midi.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitMidi::SplitMidiData SplitMidi::instance;

//---------------------------------------------------------------------------

QueueEntry<SplitMidi::EntryData> *
SplitMidi::SplitMidiData::CreateEntry(const uint8_t *data, size_t length) {
  QueueEntry<EntryData> *entry = new (length) QueueEntry<EntryData>;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void SplitMidi::SplitMidiData::Add(const uint8_t *data, size_t length) {
  QueueEntry<EntryData> *entry = CreateEntry(data, length);
  AddEntry(entry);
}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER

void SplitMidi::SplitMidiData::UpdateBuffer(TxBuffer &buffer) {
  while (head) {
    if (!buffer.Add(SplitHandlerId::MIDI, &head->data.data,
                    head->data.length)) {
      return;
    }

    RemoveHead();
  }
}

#endif

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
