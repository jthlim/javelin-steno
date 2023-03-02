//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

class Rgb {
public:
  static void SetRgb(size_t id, int r, int g, int b);
  static size_t GetCount();

  static void SetRgb_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
