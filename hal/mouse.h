//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

class Mouse {
public:
  static void PressButton(size_t index);
  static void ReleaseButton(size_t index);

  static void Move(int dx, int dy);
  static void VWheel(int delta);
  static void HWheel(int delta);
};

//---------------------------------------------------------------------------
