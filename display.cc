//---------------------------------------------------------------------------

#include "display.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Display::Clear(int displayId) {}
__attribute__((weak)) void Display::SetAutoDraw(int displayId, int autoDrawId) {
}

__attribute__((weak)) void Display::TurnOn(int displayId) {}
__attribute__((weak)) void Display::TurnOff(int displayId) {}

__attribute__((weak)) void Display::DrawPixel(int displayId, int x, int y,
                                              bool on) {}
__attribute__((weak)) void Display::DrawLine(int displayId, int x1, int y1,
                                             int x2, int y2, bool on) {}
__attribute__((weak)) void Display::DrawImage(int displayId, int x, int y,
                                              int width, int height,
                                              const uint8_t *data) {}
__attribute__((weak)) void Display::DrawText(int displayId, int x, int y,
                                             int fontId, const uint8_t *text) {}

//---------------------------------------------------------------------------
