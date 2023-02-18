//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

class Pixel {
public:
  static void SetPixel(size_t id, int r, int g, int b);
  static size_t GetCount();

  static void SetPixel_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
