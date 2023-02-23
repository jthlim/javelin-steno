//---------------------------------------------------------------------------

#include "pixel.h"
#include "console.h"
#include <string.h>

//---------------------------------------------------------------------------

__attribute__((weak)) void Pixel::SetPixel(size_t id, int r, int g, int b) {}

void Pixel::SetPixel_Binding(void *context, const char *commandLine) {
  int values[4];
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }

  for (int i = 0; i < 4; ++i) {
    ++p;

    int value = 0;
    while ('0' <= *p && *p <= '9') {
      value = 10 * value + (*p++ - '0');
    }
    values[i] = value;
  }

  SetPixel(values[0], values[1], values[2], values[3]);
  Console::SendOk();
}

__attribute__((weak)) size_t Pixel::GetCount() { return 0; }

//---------------------------------------------------------------------------