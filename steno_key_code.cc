//---------------------------------------------------------------------------

#include "steno_key_code.h"

//---------------------------------------------------------------------------

uint32_t StenoKeyCode::ResolveUnicode(uint32_t unicode, StenoCaseMode mode) {
  if (mode == StenoCaseMode::NORMAL) [[likely]] {
    return unicode;
  }

  switch (mode) {
  case StenoCaseMode::NORMAL:
    return unicode;
  case StenoCaseMode::LOWER:
  case StenoCaseMode::LOWER_ONCE:
    return Unicode::ToLower(unicode);
  case StenoCaseMode::UPPER:
  case StenoCaseMode::UPPER_ONCE:
  case StenoCaseMode::TITLE:
  case StenoCaseMode::TITLE_ONCE:
    return Unicode::ToUpper(unicode);
  default:
    __builtin_unreachable();
  }
  return unicode;
}

//---------------------------------------------------------------------------
