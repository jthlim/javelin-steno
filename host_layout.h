//---------------------------------------------------------------------------

#pragma once
#include "static_list.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum UnicodeMode : uint8_t {
  NONE,
  MACOS_UNICODE_HEX,
  WINDOWS_HEX,
  LINUX_IBUS,
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

  static void SetHostLayout_Binding(void *context, const char *commandLine);
  static void ListHostLayouts();

private:
  StaticList<const HostLayout *> layouts;

  static const HostLayout *activeLayout;
  static const HostLayouts *instance;
};

//---------------------------------------------------------------------------
