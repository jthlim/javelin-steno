//---------------------------------------------------------------------------

#include "mouse.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Mouse::PressButton(size_t index) {}
__attribute__((weak)) void Mouse::ReleaseButton(size_t index) {}
__attribute__((weak)) void Mouse::Move(int dx, int dy) {}
__attribute__((weak)) void Mouse::Wheel(int delta) {}

//---------------------------------------------------------------------------
