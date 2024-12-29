//---------------------------------------------------------------------------

#include "mouse.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Mouse::PressButton(size_t index) {}
[[gnu::weak]] void Mouse::ReleaseButton(size_t index) {}
[[gnu::weak]] void Mouse::Move(int dx, int dy) {}
[[gnu::weak]] void Mouse::Wheel(int delta) {}

//---------------------------------------------------------------------------
