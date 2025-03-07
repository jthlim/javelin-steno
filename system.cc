//---------------------------------------------------------------------------

#include "system.h"

#include "console.h"
#include "hal/external_flash.h"

//---------------------------------------------------------------------------

void StenoSystem::Print() const {
  Console::Printf("name: \"%J\"\n", name);
  Console::Printf("keys:\n");
  for (size_t i = 0; i < keys.count; ++i) {
    const StrokeKey *key = &keys[i];
    Console::Printf(" - name: \"%C\"\n", key->c);
    switch (key->type) {
    case StrokeKeyType::MASK:
      Console::Printf("   mask: 0x%x\n", key->mask);
      break;
    case StrokeKeyType::SEPARATOR:
      Console::Printf("   mask: 0\n");
      Console::Printf("   absentMask: 0x%x\n", key[0].mask);
      Console::Printf("   presentMask: 0x%x\n", key[1].mask);
      ++i;
      break;
    default:
      __builtin_unreachable();
      break;
    }
  }
  orthography.Print();
}

void StenoSystem::Print_Binding(void *context, const char *commandLine) {
  const ExternalFlashSentry sentry;
  ((StenoSystem *)context)->Print();
}

//---------------------------------------------------------------------------
