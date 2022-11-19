//---------------------------------------------------------------------------

#include "jeff_modifiers.h"
#include "../console.h"
#include "../key_code.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

constexpr StenoChord StenoJeffModifiers::TRIGGER_CHORD(ChordMask::TR |
                                                       ChordMask::ZR);
constexpr StenoChord StenoJeffModifiers::TRIGGER_MASK(ChordMask::GR |
                                                      ChordMask::TR |
                                                      ChordMask::SR |
                                                      ChordMask::DR |
                                                      ChordMask::ZR);
constexpr StenoChord StenoJeffModifiers::DATA_MASK(
    ChordMask::NUM | ChordMask::SL | ChordMask::TL | ChordMask::KL |
    ChordMask::PL | ChordMask::WL | ChordMask::HL | ChordMask::RL |
    ChordMask::A | ChordMask::O | ChordMask::STAR | ChordMask::E |
    ChordMask::U | ChordMask::LR);

constexpr StenoChord StenoJeffModifiers::IGNORE_MASK(
    ChordMask::NUM | ChordMask::SL | ChordMask::TL | ChordMask::KL |
    ChordMask::PL | ChordMask::WL | ChordMask::HL | ChordMask::RL |
    ChordMask::A | ChordMask::O | ChordMask::STAR);

constexpr StenoChord StenoJeffModifiers::CONTROL_MASK(ChordMask::FR);
constexpr StenoChord StenoJeffModifiers::SHIFT_MASK(ChordMask::RR);
constexpr StenoChord StenoJeffModifiers::SUPER_MASK(ChordMask::PR);
constexpr StenoChord StenoJeffModifiers::ALT_MASK(ChordMask::BR);

//---------------------------------------------------------------------------

struct JeffModifiersData {
  uint32_t keyState : 24;
  uint8_t keyCode : 8;
};

const JeffModifiersData DATA[] = {
    {
        .keyState = ChordMask::A,
        .keyCode = KeyCode::A,
    },
    {
        .keyState = ChordMask::PL | ChordMask::WL,
        .keyCode = KeyCode::B,
    },
    {
        .keyState = ChordMask::KL | ChordMask::RL,
        .keyCode = KeyCode::C,
    },
    {
        .keyState = ChordMask::TL | ChordMask::KL,
        .keyCode = KeyCode::D,
    },
    {
        .keyState = ChordMask::E,
        .keyCode = KeyCode::E,
    },
    {
        .keyState = ChordMask::TL | ChordMask::PL,
        .keyCode = KeyCode::F,
    },
    {
        .keyState =
            ChordMask::TL | ChordMask::KL | ChordMask::PL | ChordMask::WL,
        .keyCode = KeyCode::G,
    },
    {
        .keyState = ChordMask::HL,
        .keyCode = KeyCode::H,
    },
    {
        .keyState = ChordMask::E | ChordMask::U,
        .keyCode = KeyCode::I,
    },
    {
        .keyState =
            ChordMask::SL | ChordMask::KL | ChordMask::WL | ChordMask::RL,
        .keyCode = KeyCode::J,
    },
    {
        .keyState = ChordMask::KL,
        .keyCode = KeyCode::K,
    },
    {
        .keyState = ChordMask::HL | ChordMask::RL,
        .keyCode = KeyCode::L,
    },
    {
        .keyState = ChordMask::PL | ChordMask::HL,
        .keyCode = KeyCode::M,
    },
    {
        .keyState = ChordMask::TL | ChordMask::PL | ChordMask::HL,
        .keyCode = KeyCode::N,
    },
    {
        .keyState = ChordMask::O,
        .keyCode = KeyCode::O,
    },
    {
        .keyState = ChordMask::PL,
        .keyCode = KeyCode::P,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL,
        .keyCode = KeyCode::Q,
    },
    {
        .keyState = ChordMask::RL,
        .keyCode = KeyCode::R,
    },
    {
        .keyState = ChordMask::SL,
        .keyCode = KeyCode::S,
    },
    {
        .keyState = ChordMask::TL,
        .keyCode = KeyCode::T,
    },
    {
        .keyState = ChordMask::U,
        .keyCode = KeyCode::U,
    },
    {
        .keyState = ChordMask::SL | ChordMask::RL,
        .keyCode = KeyCode::V,
    },
    {
        .keyState = ChordMask::WL,
        .keyCode = KeyCode::W,
    },
    {
        .keyState = ChordMask::KL | ChordMask::PL,
        .keyCode = KeyCode::X,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::RL,
        .keyCode = KeyCode::Y,
    },
    {
        .keyState = ChordMask::SL | ChordMask::TL | ChordMask::KL |
                    ChordMask::PL | ChordMask::WL,
        .keyCode = KeyCode::Z,
    },

    // Navigation layer.
    {
        .keyState = ChordMask::PL | ChordMask::LR,
        .keyCode = KeyCode::UP,
    },
    {
        .keyState = ChordMask::KL | ChordMask::LR,
        .keyCode = KeyCode::LEFT,
    },
    {
        .keyState = ChordMask::WL | ChordMask::LR,
        .keyCode = KeyCode::DOWN,
    },
    {
        .keyState = ChordMask::RL | ChordMask::LR,
        .keyCode = KeyCode::RIGHT,
    },
    {
        .keyState =
            ChordMask::TL | ChordMask::PL | ChordMask::HL | ChordMask::LR,
        .keyCode = KeyCode::PAGE_UP,
    },
    {
        .keyState =
            ChordMask::KL | ChordMask::WL | ChordMask::RL | ChordMask::LR,
        .keyCode = KeyCode::PAGE_DOWN,
    },
    {
        .keyState = ChordMask::TL | ChordMask::KL | ChordMask::LR,
        .keyCode = KeyCode::HOME,
    },
    {
        .keyState = ChordMask::HL | ChordMask::RL | ChordMask::LR,
        .keyCode = KeyCode::END,
    },
    {
        .keyState = ChordMask::TL | ChordMask::LR,
        .keyCode = KeyCode::BACKSPACE,
    },
    {
        .keyState = ChordMask::HL | ChordMask::LR,
        .keyCode = KeyCode::DELETE,
    },

    // Keys layer.
    {
        .keyState =
            ChordMask::TL | ChordMask::KL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::ESC,
    },
    {
        .keyState =
            ChordMask::HL | ChordMask::RL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::TAB,
    },
    {
        .keyState = ChordMask::HL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::APOSTROPHE,
    },
    {
        .keyState = ChordMask::PL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::GRAVE,
    },
    {
        .keyState = ChordMask::KL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::DOT,
    },
    {
        .keyState = ChordMask::WL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::COMMA,
    },
    {
        .keyState =
            ChordMask::KL | ChordMask::WL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::SEMICOLON,
    },
    {
        .keyState = ChordMask::TL | ChordMask::PL | ChordMask::KL |
                    ChordMask::WL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::EQUAL,
    },
    {
        .keyState =
            ChordMask::PL | ChordMask::HL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::MINUS,
    },
    {
        .keyState =
            ChordMask::WL | ChordMask::RL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::SPACE,
    },
    {
        .keyState =
            ChordMask::KL | ChordMask::PL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::SLASH,
    },
    {
        .keyState =
            ChordMask::TL | ChordMask::WL | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::BACKSLASH,
    },
    {
        .keyState = ChordMask::TL | ChordMask::PL | ChordMask::HL |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::L_BRACKET,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::RL |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::R_BRACKET,
    },
    {
        .keyState = ChordMask::WL | ChordMask::HL | ChordMask::RL |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::ENTER,
    },

    // Number layer.
    {
        .keyState = ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_0,
    },
    {
        .keyState = ChordMask::RL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_1,
    },
    {
        .keyState = ChordMask::WL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_2,
    },
    {
        .keyState = ChordMask::WL | ChordMask::RL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_3,
    },
    {
        .keyState = ChordMask::KL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_4,
    },
    {
        .keyState = ChordMask::KL | ChordMask::RL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_5,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_6,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::RL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_7,
    },
    {
        .keyState = ChordMask::SL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_8,
    },
    {
        .keyState = ChordMask::SL | ChordMask::RL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::_9,
    },

    // Num pad numbers.
    {
        .keyState = ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_0,
    },
    {
        .keyState = ChordMask::RL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_1,
    },
    {
        .keyState = ChordMask::WL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_2,
    },
    {
        .keyState = ChordMask::WL | ChordMask::RL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_3,
    },
    {
        .keyState = ChordMask::KL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_4,
    },
    {
        .keyState = ChordMask::KL | ChordMask::RL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_5,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_6,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::RL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_7,
    },
    {
        .keyState = ChordMask::SL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_8,
    },
    {
        .keyState = ChordMask::SL | ChordMask::RL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::KP_9,
    },

    // Function key layers.
    {
        .keyState = ChordMask::RL | ChordMask::TL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F1,
    },
    {
        .keyState = ChordMask::WL | ChordMask::TL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F2,
    },
    {
        .keyState = ChordMask::WL | ChordMask::RL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F3,
    },
    {
        .keyState = ChordMask::KL | ChordMask::TL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F4,
    },
    {
        .keyState = ChordMask::KL | ChordMask::RL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F5,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F6,
    },
    {
        .keyState = ChordMask::KL | ChordMask::WL | ChordMask::RL |
                    ChordMask::TL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F7,
    },
    {
        .keyState = ChordMask::SL | ChordMask::TL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F8,
    },
    {
        .keyState = ChordMask::SL | ChordMask::RL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F9,
    },
    {
        .keyState = ChordMask::SL | ChordMask::WL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F10,
    },
    {
        .keyState = ChordMask::SL | ChordMask::WL | ChordMask::RL |
                    ChordMask::TL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F11,
    },
    {
        .keyState = ChordMask::SL | ChordMask::KL | ChordMask::TL |
                    ChordMask::PL | ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F12,
    },
    {
        .keyState = ChordMask::SL | ChordMask::KL | ChordMask::RL |
                    ChordMask::TL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F13,
    },
    {
        .keyState = ChordMask::SL | ChordMask::KL | ChordMask::WL |
                    ChordMask::TL | ChordMask::PL | ChordMask::E |
                    ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F14,
    },
    {
        .keyState = ChordMask::SL | ChordMask::KL | ChordMask::WL |
                    ChordMask::RL | ChordMask::TL | ChordMask::PL |
                    ChordMask::E | ChordMask::U | ChordMask::LR,
        .keyCode = KeyCode::F15,
    },
};

//---------------------------------------------------------------------------

void StenoJeffModifiers::Process(StenoKeyState value, StenoAction action) {
  const StenoChord chord = value.ToChord();

  bool isModifier = (chord & TRIGGER_MASK) == TRIGGER_CHORD;

  switch (action) {
  case StenoAction::PRESS:
    if (isModifier) {
      if (!wasModifier) {
        wasModifier = true;
        nextProcessor.Process(value, StenoAction::CANCEL);
      }
      UpdateModifiers(chord);
      return;
    } else if (wasModifier) {
      wasModifier = false;
      UpdateModifiers(StenoChord());
    }
    break;

  case StenoAction::RELEASE:
    if (wasModifier) {
      if (value.IsEmpty()) {
        wasModifier = false;
      }
      UpdateModifiers(chord);
      return;
    }
    break;

  case StenoAction::CANCEL:
    wasModifier = false;
    UpdateModifiers(StenoChord());
    break;

  case StenoAction::TRIGGER:
    if (wasModifier || isModifier) {
      StenoChord previousChord = lastModifiers;
      UpdateModifiers(chord);
      bool result = TriggerSendKey(chord);
      UpdateModifiers(previousChord);
      if (result) {
        return;
      }
    }
    break;
  }
  nextProcessor.Process(value, action);
}

void StenoJeffModifiers::PrintInfo() const {
  Console::Printf("  Jeff's Modifiers\n");
  nextProcessor.PrintInfo();
}

void StenoJeffModifiers::UpdateModifiers(StenoChord chord) {
  lastModifiers = chord;
  bool isControl = (chord & CONTROL_MASK).IsNotEmpty();
  if (isControl != wasControl) {
    wasControl = isControl;
    if (isControl) {
      Key::Press(KeyCode::L_CTRL);
    } else {
      Key::Release(KeyCode::L_CTRL);
    }
  }

  bool isShift = (chord & SHIFT_MASK).IsNotEmpty();
  if (isShift != wasShift) {
    wasShift = isShift;
    if (isShift) {
      Key::Press(KeyCode::L_SHIFT);
    } else {
      Key::Release(KeyCode::L_SHIFT);
    }
  }

  bool isSuper = (chord & SUPER_MASK).IsNotEmpty();
  if (isSuper != wasSuper) {
    wasSuper = isSuper;
    if (isSuper) {
      Key::Press(KeyCode::L_META);
    } else {
      Key::Release(KeyCode::L_META);
    }
  }

  bool isAlt = (chord & ALT_MASK).IsNotEmpty();
  if (isAlt != wasAlt) {
    wasAlt = isAlt;
    if (isAlt) {
      Key::Press(KeyCode::L_ALT);
    } else {
      Key::Release(KeyCode::L_ALT);
    }
  }
}

bool StenoJeffModifiers::TriggerSendKey(StenoChord chord) const {
  const uint32_t keyState = (chord & DATA_MASK).GetKeyState();

  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    if (DATA[i].keyState == keyState) {
      uint8_t code = DATA[i].keyCode;
      Key::Press(code);
      Key::Release(code);
      return true;
    }
  }

  // Don't trigger if only the modifier keys and/or layer keys have been
  // pressed.
  return (chord & IGNORE_MASK).IsEmpty();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../key_code.h"
#include "../unit_test.h"
#include "fake_processor.h"

TEST_BEGIN("JeffModifiers tests") {
  FakeStenoProcessor fakeProcessor;
  StenoJeffModifiers jeffModifier(fakeProcessor);
  StenoProcessor processor(jeffModifier);

  processor.Process(StenoKey::TR, true);
  processor.Process(StenoKey::ZR, true);
  processor.Process(StenoKey::PR, true);
  assert(Key::history.size() == 1);
  assert(Key::history[0].isPress == true);
  assert(Key::history[0].code == KeyCode::L_META);
}
TEST_END

//---------------------------------------------------------------------------
