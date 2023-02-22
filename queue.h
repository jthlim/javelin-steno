//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T> struct QueueEntry {
  QueueEntry *next;

  T data;
};

template <typename T> class Queue {
public:
  Queue() : head(nullptr), tail(&head) {}

  void AddEntry(QueueEntry<T> *entry) {
    *tail = entry;
    tail = &entry->next;
  }

protected:
  QueueEntry<T> *head;
  QueueEntry<T> **tail;
};

//---------------------------------------------------------------------------
