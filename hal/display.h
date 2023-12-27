//---------------------------------------------------------------------------

#pragma once
#include "../font/text_alignment.h"
#include <stdint.h>

//---------------------------------------------------------------------------

enum FontId : int;

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
                        const uint8_t *data);
  static void DrawGrayscaleRange(int displayId, int x, int y, int width,
                                 int height, const uint8_t *data, int min,
                                 int max);
  static void DrawText(int displayId, int x, int y, FontId fontId,
                       TextAlignment alignment, const char *text);
  static void DrawRect(int displayId, int left, int top, int right, int bottom);

  static void SetDrawColor(int displayId, int color);
};

//---------------------------------------------------------------------------
