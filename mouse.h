//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

class Mouse {
public:
  static void PressButton(size_t index);
  static void ReleaseButton(size_t index);

  static void Move(int dx, int dy);
  static void Wheel(int delta);
};

//---------------------------------------------------------------------------
