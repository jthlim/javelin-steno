//---------------------------------------------------------------------------

#pragma once
#include "container/static_list.h"
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

enum UnicodeMode : uint8_t {
  NONE,
  LINUX_IBUS,
  MACOS_UNICODE_HEX,
  WIN_COMPOSE,
  WINDOWS_HEX,
};

//---------------------------------------------------------------------------

struct HostLayoutEntry {
  uint32_t unicode : 24;
  uint32_t length : 8;
  uint16_t keyCodes[6];
};
static_assert(sizeof(HostLayoutEntry) == 16,
              "Expect HostLayoutEntry to be 16 bytes");

//---------------------------------------------------------------------------

// This is used to translate steno unicode -> scan codes.
struct HostLayout {
  const char name[15];
  UnicodeMode unicodeMode;
  uint16_t asciiKeyCodes[128];
  StaticList<HostLayoutEntry> entries;

  const char *GetName() const { return name; }
  const HostLayoutEntry *GetSequenceForUnicode(uint32_t unicode) const;
  uint32_t GetUnicodeForScancode(uint32_t scanCode) const;

  static const HostLayout ansi;
};

//---------------------------------------------------------------------------

class HostLayouts {
public:
  static void SetData(const HostLayouts &layouts);

  static void SetActiveLayout(const HostLayout &layout) {
    activeLayout = &layout;
  }
  static bool SetActiveLayout(const char *name);
  static const HostLayout &GetActiveLayout() { return *activeLayout; }

  static void AddConsoleCommands(Console &console);

  static void ListHostLayouts();
  static void GetHostLayout();

private:
  StaticList<const HostLayout *> layouts;

  static const HostLayout *activeLayout;
  static const HostLayouts *instance;

  static void SetHostLayout_Binding(void *context, const char *commandLine);
  static void DumpHostLayout_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
