//---------------------------------------------------------------------------

#include "host_layout.h"
#include "console.h"
#include "hal/external_flash.h"
#include "key_code.h"
#include "steno_key_code_emitter.h"
#include "str.h"
#include "unicode_script.h"

//---------------------------------------------------------------------------

#define ENABLE_DUMP_HOST_LAYOUT 0

//---------------------------------------------------------------------------

constexpr HostLayout HostLayout::ansi =
    {
        .name = "us",
        .scriptOffset = 0,
        .asciiKeyCodes =
            {
// clang-format off
// spellchecker: disable
#define LS MODIFIER_L_SHIFT_FLAG
#define BS BACKSPACE
#define APOS APOSTROPHE
#define SCN SEMICOLON
  // NUL       SOH           STX             ETX           EOT    ENQ    ACK    BEL
  0,           0,            0,              0,            0,     0,     0,     0,
  // BS        TAB           LF              VT            FF     CR     SO     SI
  KeyCode::BS, KeyCode::TAB, KeyCode::ENTER, 0,            0,     0,     0,     0,
  // DLE       DC1           DC2             DC3           DC4    NAK    SYN    ETB
  0,           0,            0,              0,            0,     0,     0,     0,
  // CAN       EM            SUB             ESC           FS     GS     RS     US
  0,           0,            0,              KeyCode::ESC, 0,     0,     0,     0,

  //              !               "               #               $               %               &               '
  KeyCode::SPACE, LS|KeyCode::_1, LS|KeyCode::APOS,LS|KeyCode::_3,   LS|KeyCode::_4,   LS|KeyCode::_5,   LS|KeyCode::_7,   KeyCode::APOSTROPHE,
  // (            )               *               +               ,               -               .               /
  LS|KeyCode::_9, LS|KeyCode::_0, LS|KeyCode::_8, LS|KeyCode::EQUAL, KeyCode::COMMA,  KeyCode::MINUS,  KeyCode::DOT,   KeyCode::SLASH,
  // 0            1               2               3               4               5               6               7
  KeyCode::_0,    KeyCode::_1,    KeyCode::_2,    KeyCode::_3,     KeyCode::_4,     KeyCode::_5,     KeyCode::_6,     KeyCode::_7,
  // 8            9               :               ;               <               =               >               ?
  KeyCode::_8,    KeyCode::_9,    LS|KeyCode::SCN,KeyCode::SCN,   LS|KeyCode::COMMA,KeyCode::EQUAL,   LS|KeyCode::DOT, LS|KeyCode::SLASH,
  // @            A               B               C               D               E               F               G
  LS|KeyCode::_2, LS|KeyCode::A,  LS|KeyCode::B,  LS|KeyCode::C,  LS|KeyCode::D,   LS|KeyCode::E,   LS|KeyCode::F,   LS|KeyCode::G,
  // H            I               J               K               L               M               N               O
  LS|KeyCode::H,  LS|KeyCode::I,  LS|KeyCode::J,  LS|KeyCode::K,  LS|KeyCode::L,   LS|KeyCode::M,   LS|KeyCode::N,   LS|KeyCode::O,
  // P            Q               R               S               T               U               V               W
  LS|KeyCode::P,  LS|KeyCode::Q,  LS|KeyCode::R,  LS|KeyCode::S,  LS|KeyCode::T,   LS|KeyCode::U,   LS|KeyCode::V,   LS|KeyCode::W,
  // X            Y               Z               [               \               ]               ^               _
  LS|KeyCode::X,  LS|KeyCode::Y,  LS|KeyCode::Z,  KeyCode::L_BRACKET,  KeyCode::BACKSLASH,  KeyCode::R_BRACKET,  LS|KeyCode::_6,   LS|KeyCode::MINUS,
  // `            a               b               c               d               e               f               g
  KeyCode::GRAVE, KeyCode::A,     KeyCode::B,     KeyCode::C,     KeyCode::D,     KeyCode::E,     KeyCode::F,     KeyCode::G,
  // h            i               j               k               l               m               n               o
  KeyCode::H,     KeyCode::I,     KeyCode::J,     KeyCode::K,     KeyCode::L,     KeyCode::M,     KeyCode::N,     KeyCode::O,
  // p            q               r               s               t               u               v               w
  KeyCode::P,     KeyCode::Q,     KeyCode::R,     KeyCode::S,     KeyCode::T,     KeyCode::U,     KeyCode::V,     KeyCode::W,
  // x            y               z               {               |               }               ~               DEL
  KeyCode::X,     KeyCode::Y,     KeyCode::Z,     LS|KeyCode::L_BRACKET,LS|KeyCode::BACKSLASH,LS|KeyCode::R_BRACKET,LS|KeyCode::GRAVE, 0,
},
#undef LS
#undef BS
#undef APOS
#undef SCN
// spellchecker: enable
        // clang-format on
};

const HostLayout *HostLayouts::activeLayout = &HostLayout::ansi;
const HostLayouts *HostLayouts::instance;

//---------------------------------------------------------------------------

const HostLayoutEntry *
HostLayout::GetSequenceForUnicode(uint32_t unicode) const {
  const HostLayoutEntry *left = begin(entries);
  const HostLayoutEntry *right = end(entries);

  while (left < right) {
    const HostLayoutEntry *mid = left + size_t(right - left) / 2;

    const int compare = (int)unicode - (int)mid->unicode;
    if (compare < 0) {
      right = mid;
    } else if (compare == 0) {
      return mid;
    } else {
      left = mid + 1;
    }
  }
  return nullptr;
}

uint32_t HostLayout::GetUnicodeForScancode(uint32_t scanCode) const {
  for (size_t i = 0; i < sizeof(asciiKeyCodes) / sizeof(*asciiKeyCodes); ++i) {
    if (asciiKeyCodes[i] == scanCode) {
      return (uint32_t)i;
    }
  }
  return 0;
}

const ScriptByteCode *HostLayout::GetScript() const {
  if (scriptOffset == 0) {
    return (const ScriptByteCode *)UnicodeScript::EMPTY_SCRIPT;
  }
  return (const ScriptByteCode *)(intptr_t(this) + scriptOffset);
}

//---------------------------------------------------------------------------

void HostLayouts::SetData(const HostLayouts &layouts) {
  instance = &layouts;
  SetActiveLayout(*layouts.layouts.Front());
}

//---------------------------------------------------------------------------

void HostLayouts::SetActiveLayout(const HostLayout &layout) {
  activeLayout = &layout;
  UnicodeScript::instance.SetScript(activeLayout->GetScript());
  UnicodeScript::instance.ExecuteInitScript();
}

bool HostLayouts::SetActiveLayout(const char *name) {
  for (const HostLayout *layout : instance->layouts) {
    if (Str::Eq(name, layout->GetName())) {
      SetActiveLayout(*layout);
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------

void HostLayouts::SetHostLayout_Binding(void *context,
                                        const char *commandLine) {
  const char *layoutName = strchr(commandLine, ' ');
  if (!layoutName) {
    Console::Printf("ERR No host layout specified\n\n");
    return;
  }
  ++layoutName;

  const ExternalFlashSentry sentry;
  if (SetActiveLayout(layoutName)) {
    Console::SendOk();
  } else {
    Console::Printf("ERR Unable to set host layout: \"%s\"\n\n", layoutName);
  }
}

void HostLayouts::DumpHostLayout_Binding(void *context,
                                         const char *commandLine) {
  const ExternalFlashSentry sentry;
  Console::Printf("HostLayout: %s\n", activeLayout->GetName());
  for (size_t i = 0; i < 128; ++i) {
    if (i % 16 == 0)
      Console::Printf("\n%04zx:", i);
    Console::Printf(" %04x", activeLayout->asciiKeyCodes[i]);
  }
  Console::Printf("\nUnicodeEntries: %zu", activeLayout->entries.GetCount());

  for (const HostLayoutEntry &entry : activeLayout->entries) {
    Console::Printf("\n\"%C\": ", entry.unicode);
    for (size_t i = 0; i < entry.length; ++i) {
      Console::Printf(" %04x", entry.keyCodes[i]);
    }
  }

  Console::Printf("\n\n");
}

void HostLayouts::ListHostLayouts() {
  const ExternalFlashSentry sentry;

  Console::Printf("[");
  const char *format = ",%Y";
  ++format;
  for (const HostLayout *layout : instance->layouts) {
    Console::Printf(format, layout->GetName());
    format = ",%Y";
  }
  Console::Printf("]\n\n");
}

void HostLayouts::GetHostLayout() {
  const ExternalFlashSentry sentry;
  Console::Printf("%s\n\n", HostLayouts::GetActiveLayout().GetName());
}

//---------------------------------------------------------------------------

void HostLayouts::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_host_layout", "Sets the current host layout",
                          &HostLayouts::SetHostLayout_Binding, nullptr);

#if ENABLE_DUMP_HOST_LAYOUT
  console.RegisterCommand("dump_host_layout", "Dumps the current host layout",
                          &HostLayouts::DumpHostLayout_Binding, nullptr);
#endif
}

//---------------------------------------------------------------------------
