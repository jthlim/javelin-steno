//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class Display {
public:
  static void Clear(int displayId);
  static void SetAutoDraw(int displayId, int autoDrawId);

  static void TurnOn(int displayId);
  static void TurnOff(int displayId);

  static void DrawPixel(int displayId, int x, int y, bool on);
  static void DrawLine(int displayId, int x1, int y1, int x2, int y2, bool on);
  static void DrawImage(int displayId, int x, int y, int width, int height,
                        const uint8_t *data);
  static void DrawText(int displayId, int x, int y, int fontId,
                       const uint8_t *text);
};

//---------------------------------------------------------------------------
