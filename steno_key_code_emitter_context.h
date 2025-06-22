//---------------------------------------------------------------------------

#pragma once
#include "key.h"
#include "steno_key_code.h"
#include "steno_key_code_emitter.h"

//---------------------------------------------------------------------------

struct HostLayoutEntry;
struct HostLayout;

//---------------------------------------------------------------------------

struct StenoKeyCodeEmitter::EmitterContext {
  EmitterContext();

  uint32_t modifiers = 0;
  bool shouldCombineUndo = true;
  bool hasDeterminedNumLockState = false;
  bool isNumLockOn;
  const HostLayout &hostLayout;

  static const KeyCode::Value MASK_KEY_CODES[];
  static const KeyCode::Value HEX_KEY_CODES[];
  static const uint16_t ALT_HEX_KEY_CODES[];
  static const uint16_t KP_ALT_HEX_KEY_CODES[];

  bool GetIsNumLockOn();
  void ProcessStenoKeyCode(StenoKeyCode stenoKeyCode);

  static void PressKey(KeyCode keyCode) { Key::Press(keyCode); }
  static void ReleaseKey(KeyCode keyCode) { Key::Release(keyCode); };
  static void TapKey(KeyCode keyCode) { Key::Tap(keyCode); }

  void EmitKeyCode(uint32_t keyCode);

  void PressModifiers(uint32_t modifiers);
  void ReleaseModifiers(uint32_t modifiers);

  void EmitAscii(uint32_t unicode);
  void EmitNonAscii(uint32_t unicode);
  void EmitSequence(const HostLayoutEntry &sequence);
  void EmitMacOsUnicodeHex(uint32_t unicode);
  void EmitIBus(uint32_t unicode);
  void EmitWinCompose(uint32_t unicode);
  void RecurseEmitHex(uint32_t unicode);
  static void EmitIBusDelay();
  void EmitWindowsHex(uint32_t unicode);
  void EmitUCS2AltHex(uint32_t unicode);
};

//---------------------------------------------------------------------------
