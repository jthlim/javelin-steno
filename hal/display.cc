//---------------------------------------------------------------------------

#include "display.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Display::Clear(int displayId) {}
[[gnu::weak]] void Display::SetAutoDraw(int displayId, int autoDrawId) {}

[[gnu::weak]] void Display::SetScreenOn(int displayId, bool on) {}
[[gnu::weak]] void Display::SetContrast(int displayId, int contrast) {}

[[gnu::weak]] void Display::DrawPixel(int displayId, int x, int y) {}
[[gnu::weak]] void Display::DrawLine(int displayId, int x1, int y1, int x2,
                                     int y2) {}
[[gnu::weak]] void Display::DrawImage(int displayId, int x, int y, int width,
                                      int height, const uint8_t *data) {}
[[gnu::weak]] void Display::DrawGrayscaleRange(int displayId, int x, int y,
                                               int width, int height,
                                               const uint8_t *data, int min,
                                               int max) {}
[[gnu::weak]] void Display::DrawText(int displayId, int x, int y, FontId fontId,
                                     TextAlignment alignment,
                                     const char *text) {}
[[gnu::weak]] void Display::DrawRect(int displayId, int left, int top,
                                     int right, int bottom) {}
[[gnu::weak]] void Display::SetDrawColor(int displayId, int color) {}

//---------------------------------------------------------------------------
