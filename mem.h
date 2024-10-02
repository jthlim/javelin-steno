//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

template <size_t ALIGN> struct AlignedMem {};

template <> struct AlignedMem<1> {
  static void Clear(void *p, size_t length) { memset(p, 0, length); }
  static bool ConstantTimeEq(const void *p, const void *q, size_t length);
  static void Copy(void *d, const void *s, size_t length) {
    memcpy(d, s, length);
  }
  static bool Eq(const void *p, const void *q, size_t length) {
    return memcmp(p, q, length) == 0;
  }
  static void Fill(void *p, size_t length) { memset(p, 0xff, length); }
};

template <> struct AlignedMem<sizeof(size_t)> {
  static void Clear(void *p, size_t length);
  static bool ConstantTimeEq(const void *p, const void *q, size_t length);
  static void Copy(void *d, const void *s, size_t length);
  static bool Eq(const void *p, const void *q, size_t length);
  static void Fill(void *p, size_t length);
};

template <typename T> struct TypedMem {
  static void Clear(T *data, size_t length) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Clear(
        data, length);
  }
  static bool ConstantTimeEq(const T *p, const T *q, size_t length) {
    return AlignedMem < alignof(T) >= sizeof(size_t)
               ? sizeof(size_t)
               : 1 > ::ConstantTimeEq(p, q, length);
  }
  static void Copy(T *d, const T *s, size_t length) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Copy(d, s,
                                                                        length);
  }
  static bool Eq(const T *p, const T *q, size_t length) {
    return AlignedMem < alignof(T) >= sizeof(size_t) ? sizeof(size_t)
                                                     : 1 > ::Eq(p, q, length);
  }
  static void Fill(T *data, size_t length) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Fill(data,
                                                                        length);
  }
};

struct Mem {
  template <typename T> static void Clear(T *data, size_t length) {
    TypedMem<T>::Clear(data, length);
  }
  template <typename T> static void Clear(T &data) { Clear(&data, sizeof(T)); }
  template <typename T>
  static bool ConstantTimeEq(const T *p, const T *q, size_t length) {
    return TypedMem<T>::ConstantTimeEq(p, q, length);
  }
  template <typename T> static void Copy(T *d, const T *s, size_t length) {
    TypedMem<T>::Copy(d, s, length);
  }
  template <typename T> static bool Eq(const T *p, const T *q, size_t length) {
    return TypedMem<T>::Eq(p, q, length);
  }
  template <typename T> static void Fill(T *data, size_t length) {
    TypedMem<T>::Fill(data, length);
  }
  template <typename T> static void Fill(T &data) { Fill(&data, sizeof(T)); }
};

//---------------------------------------------------------------------------
