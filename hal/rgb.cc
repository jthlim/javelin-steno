//---------------------------------------------------------------------------

#include "rgb.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

void Rgb::SetRgb_Binding(void *context, const char *commandLine) {
  int values[4];
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }

  for (int i = 0; i < 4; ++i) {
    p = Str::ParseInteger(&values[i], p + 1, false);

    if (!p) {
      Console::Printf("ERR Missing parameters\n\n");
    }
  }

  SetRgb(values[0], values[1], values[2], values[3]);
  Console::SendOk();
}

void Rgb::SetHsv(size_t id, int h, int s, int v) {
  h = 6 * (h & 0xffff);

  int C = v * s >> 8;
  int X;
  int m;

  if (h & 0x10000) {
    X = C * (0x10000 - (h & 0xffff)) >> 16;
    m = v - C;

    int temp = C;
    C = X;
    X = temp;
  } else {
    X = C * (h & 0xffff) >> 16;
    m = v - C;
  }

  int r = 0;
  int g = 0;
  int b = 0;

  switch (h >> 17) {
  case 0:
    r = C;
    g = X;
    break;
  case 1:
    g = C;
    b = X;
    break;
  case 2:
    b = C;
    r = X;
    break;
  default:
    __builtin_unreachable();
  }

  SetRgb(id, r + m, g + m, b + m);
}

__attribute__((weak)) void Rgb::SetRgb(size_t id, int r, int g, int b) {}

__attribute__((weak)) size_t Rgb::GetCount() { return 0; }

//---------------------------------------------------------------------------

#if JAVELIN_RGB
void Rgb::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_rgb", "Sets a single RGB (index, r, g, b)",
                          &SetRgb_Binding, nullptr);
}
#endif

//---------------------------------------------------------------------------
