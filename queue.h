//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

template <typename T> struct QueueEntry {
  QueueEntry *next;

  T data;

  static void *operator new(size_t n) noexcept { return malloc(n); }
  static void *operator new(size_t n, size_t extra) noexcept {
    return malloc(n + extra);
  }
  static void operator delete(void *p) noexcept { free(p); }
  static void operator delete(void *p, size_t extra) noexcept { free(p); }
};

template <typename T> class Queue {
public:
  Queue() : head(nullptr), tail(&head) {}

  void AddEntry(QueueEntry<T> *entry) {
    *tail = entry;
    tail = &entry->next;
  }

  void RemoveHead() {
    QueueEntry<T> *entry = head;
    head = entry->next;
    if (head == nullptr) {
      tail = &head;
    }
    delete entry;
  }

protected:
  QueueEntry<T> *head;
  QueueEntry<T> **tail;
};

//---------------------------------------------------------------------------
