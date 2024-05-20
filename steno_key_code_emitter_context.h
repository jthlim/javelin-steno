//---------------------------------------------------------------------------

#pragma once
#include "key.h"
#include "steno_key_code.h"
#include "steno_key_code_emitter.h"

//---------------------------------------------------------------------------

struct StenoKeyCodeEmitter::EmitterContext {
  uint32_t modifiers = 0;
  bool shouldCombineUndo = true;
  bool hasDeterminedNumLockState = false;
  bool isNumLockOn;

  static const KeyCode::Value MASK_KEY_CODES[];
  static const KeyCode::Value HEX_KEY_CODES[];
  static const uint16_t ALT_HEX_KEY_CODES[];
  static const uint16_t KP_ALT_HEX_KEY_CODES[];
  static const uint16_t ASCII_KEY_CODES[];

  bool GetIsNumLockOn();
  void ProcessStenoKeyCode(StenoKeyCode stenoKeyCode);

  static void PressKey(KeyCode keyCode) { Key::Press(keyCode); }
  static void ReleaseKey(KeyCode keyCode) { Key::Release(keyCode); };

  static void TapKey(KeyCode keyCode) {
    PressKey(keyCode);
    ReleaseKey(keyCode);
  }

  void EmitKeyCode(uint32_t keyCode);

  static void PressModifiers(uint32_t modifiers);
  static void ReleaseModifiers(uint32_t modifiers);

  void EmitNonAscii(uint32_t unicode);
  void EmitMacOsUs(uint32_t unicode);
  void EmitMacOsUnicodeHex(uint32_t unicode);
  void EmitWindowsAlt(uint32_t unicode);
  void RecurseEmitWindowsAlt(uint32_t alt);
  void EmitIBus(uint32_t unicode);
  void RecurseEmitIBus(uint32_t unicode);
  static void EmitIBusDelay();
  void EmitWindowsHex(uint32_t unicode);
  void EmitUCS2AltHex(uint32_t unicode);
};

//---------------------------------------------------------------------------
