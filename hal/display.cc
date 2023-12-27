//---------------------------------------------------------------------------

#include "display.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Display::Clear(int displayId) {}
__attribute__((weak)) void Display::SetAutoDraw(int displayId, int autoDrawId) {
}

__attribute__((weak)) void Display::SetScreenOn(int displayId, bool on) {}
__attribute__((weak)) void Display::SetContrast(int displayId, int constrast) {}

__attribute__((weak)) void Display::DrawPixel(int displayId, int x, int y) {}
__attribute__((weak)) void Display::DrawLine(int displayId, int x1, int y1,
                                             int x2, int y2) {}
__attribute__((weak)) void Display::DrawImage(int displayId, int x, int y,
                                              int width, int height,
                                              const uint8_t *data) {}
__attribute__((weak)) void
Display::DrawGrayscaleRange(int displayId, int x, int y, int width, int height,
                            const uint8_t *data, int min, int max) {}
__attribute__((weak)) void Display::DrawText(int displayId, int x, int y,
                                             FontId fontId,
                                             TextAlignment alignment,
                                             const char *text) {}
__attribute__((weak)) void Display::DrawRect(int displayId, int left, int top,
                                             int right, int bottom) {}
__attribute__((weak)) void Display::SetDrawColor(int displayId, int color) {}

//---------------------------------------------------------------------------
