//---------------------------------------------------------------------------

#pragma once
#include <string.h>

//---------------------------------------------------------------------------

template <size_t ALIGN> struct AlignedMem {};

template <> struct AlignedMem<1> {
  static void Clear(void *p, size_t byteLength) { memset(p, 0, byteLength); }
  static bool ConstantTimeEq(const void *p, const void *q, size_t byteLength);
  static void Copy(void *d, const void *s, size_t byteLength) {
    memcpy(d, s, byteLength);
  }
  static bool Eq(const void *p, const void *q, size_t byteLength) {
    return memcmp(p, q, byteLength) == 0;
  }
  static void Fill(void *p, size_t byteLength) { memset(p, 0xff, byteLength); }
};

template <> struct AlignedMem<sizeof(size_t)> {
  static void Clear(void *p, size_t byteLength);
  static bool ConstantTimeEq(const void *p, const void *q, size_t byteLength);
  static void Copy(void *d, const void *s, size_t byteLength);
  static bool Eq(const void *p, const void *q, size_t byteLength);
  static void Fill(void *p, size_t byteLength);
};

template <typename T> struct TypedMem {
  static void Clear(T *data, size_t byteLength) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Clear(
        data, byteLength);
  }
  static bool ConstantTimeEq(const T *p, const T *q, size_t byteLength) {
    return AlignedMem < alignof(T) >= sizeof(size_t)
               ? sizeof(size_t)
               : 1 > ::ConstantTimeEq(p, q, byteLength);
  }
  static void Copy(T *d, const T *s, size_t byteLength) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Copy(
        d, s, byteLength);
  }
  static bool Eq(const T *p, const T *q, size_t byteLength) {
    return AlignedMem < alignof(T) >= sizeof(size_t)
               ? sizeof(size_t)
               : 1 > ::Eq(p, q, byteLength);
  }
  static void Fill(T *data, size_t byteLength) {
    AlignedMem<alignof(T) >= sizeof(size_t) ? sizeof(size_t) : 1>::Fill(
        data, byteLength);
  }
};

struct Mem {
  template <typename T> static void Clear(T *data, size_t byteLength) {
    TypedMem<T>::Clear(data, byteLength);
  }
  template <typename T> static void Clear(T &data) { Clear(&data, sizeof(T)); }
  template <typename T>
  static bool ConstantTimeEq(const T *p, const T *q, size_t byteLength) {
    return TypedMem<T>::ConstantTimeEq(p, q, byteLength);
  }
  template <typename T> static void Copy(T *d, const T *s, size_t byteLength) {
    TypedMem<T>::Copy(d, s, byteLength);
  }
  template <typename T>
  static bool Eq(const T *p, const T *q, size_t byteLength) {
    return TypedMem<T>::Eq(p, q, byteLength);
  }
  template <typename T> static void Fill(T *data, size_t byteLength) {
    TypedMem<T>::Fill(data, byteLength);
  }
  template <typename T> static void Fill(T &data) { Fill(&data, sizeof(T)); }
};

//---------------------------------------------------------------------------
