//---------------------------------------------------------------------------

#pragma once
#include "../font/text_alignment.h"
#include <stdint.h>

//---------------------------------------------------------------------------

enum FontId : int;

//---------------------------------------------------------------------------

enum class ImageFormat : uint8_t {
  BITMAP,
  LUMINANCE8,
  RGB332,
  RGB565,
  RGB888,
  ALPHA8,
  ARGB1555,
  RGBA8888,
};

//---------------------------------------------------------------------------

class Display {
public:
  static void Clear(int displayId);
  static void SetAutoDraw(int displayId, int autoDrawId);

  static void SetScreenOn(int displayId, bool on);
  static void SetContrast(int displayId, int contrast);

  static void DrawPixel(int displayId, int x, int y);
  static void DrawLine(int displayId, int x1, int y1, int x2, int y2);
  static void DrawImage(int displayId, int x, int y, int width, int height,
                        ImageFormat format, const uint8_t *data);
  static void DrawLuminanceRange(int displayId, int x, int y, int width,
                                 int height, const uint8_t *data, int min,
                                 int max);
  static void DrawText(int displayId, int x, int y, FontId fontId,
                       TextAlignment alignment, const char *text);
  static void DrawRect(int displayId, int left, int top, int right, int bottom);

  static void SetDrawColor(int displayId, int color);
  static void SetDrawColorRgb(int displayId, int r, int g, int b);
  static void SetDrawColorHsv(int displayId, int h, int s, int v);

  static void DrawEffect(int displayId, int effectId, int parameter);
};

//---------------------------------------------------------------------------