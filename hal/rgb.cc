//---------------------------------------------------------------------------

#include "rgb.h"
#include "../base64.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

void Rgb::SetRgb_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }

  int startRgbId;
  p = Str::ParseInteger(&startRgbId, p + 1, false);
  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }

  if (Base64::IsValid(p + 1)) {
    SetRgbBase64(startRgbId, (uint8_t *)p + 1);
    Console::SendOk();
    return;
  }

  int values[3];
  for (int i = 0; i < 3; ++i) {
    p = Str::ParseInteger(&values[i], p + 1, false);

    if (!p) {
      Console::Printf("ERR Missing parameters\n\n");
      return;
    }
  }

  SetRgb(startRgbId, values[0], values[1], values[2]);
  Console::SendOk();
}

void Rgb::SetRgbBase64(size_t startRgbId, const uint8_t *p) {
  uint8_t buffer[256];
  const size_t byteCount = Base64::Decode(buffer, (uint8_t *)p);
  const size_t rgbCount = byteCount / 3;
  const uint8_t *rgb = buffer;
  for (int i = 0; i < rgbCount; ++i, rgb += 3) {
    SetRgb(startRgbId + i, rgb[0], rgb[1], rgb[2]);
  }
}

void Rgb::SetHsv(size_t id, int h, int s, int v) {
  h = 6 * (h & 0xffff);

  int C = v * s >> 8;
  int X;
  int m;

  if (h & 0x10000) {
    X = C * (0x10000 - (h & 0xffff)) >> 16;
    m = v - C;

    const int temp = C;
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

[[gnu::weak]] void Rgb::SetRgb(size_t id, int r, int g, int b) {}

[[gnu::weak]] size_t Rgb::GetCount() { return 0; }

//---------------------------------------------------------------------------

#if JAVELIN_RGB
void Rgb::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_rgb", "Sets a single RGB (index, r, g, b)",
                          &SetRgb_Binding, nullptr);
}
#endif

//---------------------------------------------------------------------------
