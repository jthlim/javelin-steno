//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct KeyboardLayoutTable {
  uint8_t values[64];
};

class KeyboardLayout {
public:
  enum Value : uint8_t {
    QWERTY,
    DVORAK,
    COLEMAK,
    WORKMAN,
    QGMLWY,
    COUNT,
  };

  KeyboardLayout() = default;
  KeyboardLayout(Value value) : value(value) {}

  const char *GetName() const { return NAMES[size_t(value)]; }

  static bool SetActiveLayout(const char *name);

  static void SetActiveLayout(KeyboardLayout layout) {
    activeInstance.layout = layout.value;
    activeInstance.layoutTable = LAYOUT_TABLES[size_t(layout.value)];
  }

  static KeyboardLayout GetActiveLayout() { return activeInstance.layout; }

  static const KeyboardLayoutTable *GetActiveLayoutTable() {
    return activeInstance.layoutTable;
  }

  static void SetKeyboardLayout_Binding(void *context, const char *commandLine);

private:
  Value value;

  struct ActiveInstance {
    Value layout;
    const KeyboardLayoutTable *layoutTable;
  };
  static ActiveInstance activeInstance;

  static const char *const NAMES[];
  static const KeyboardLayoutTable *const LAYOUT_TABLES[];
};

//---------------------------------------------------------------------------
