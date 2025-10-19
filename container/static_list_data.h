//---------------------------------------------------------------------------

#pragma once
#include "static_list.h"

//---------------------------------------------------------------------------

template <typename T, size_t CAPACITY> struct StaticListData {
  StaticList<T, CAPACITY> &AsStaticList() {
    return *(StaticList<T, CAPACITY> *)this;
  }

  const StaticList<T, CAPACITY> &AsStaticList() const {
    return *(const StaticList<T, CAPACITY> *)this;
  }

  size_t count = 0;
  T data[CAPACITY];
};

//---------------------------------------------------------------------------
