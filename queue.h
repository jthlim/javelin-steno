//---------------------------------------------------------------------------

#pragma once
#include "malloc_allocate.h"

//---------------------------------------------------------------------------

template <typename T> struct QueueEntry : public JavelinMallocAllocate {
  QueueEntry *next;

  T data;

  using JavelinMallocAllocate::operator new;
  using JavelinMallocAllocate::operator delete;

  static void *operator new(size_t n, size_t extra) noexcept {
    return operator new(n + extra);
  }
  static void operator delete(void *p, size_t extra) noexcept {
    operator delete(p);
  }
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
