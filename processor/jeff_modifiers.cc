//---------------------------------------------------------------------------

#include "jeff_modifiers.h"
#include "../console.h"
#include "../key_code.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

constexpr StenoStroke StenoJeffModifiers::TRIGGER_STROKE(StrokeMask::TR |
                                                         StrokeMask::ZR);
constexpr StenoStroke StenoJeffModifiers::TRIGGER_MASK(StrokeMask::GR |
                                                       StrokeMask::TR |
                                                       StrokeMask::SR |
                                                       StrokeMask::DR |
                                                       StrokeMask::ZR);
constexpr StenoStroke StenoJeffModifiers::DATA_MASK(
    StrokeMask::NUM | StrokeMask::SL | StrokeMask::TL | StrokeMask::KL |
    StrokeMask::PL | StrokeMask::WL | StrokeMask::HL | StrokeMask::RL |
    StrokeMask::A | StrokeMask::O | StrokeMask::STAR | StrokeMask::E |
    StrokeMask::U | StrokeMask::LR);

constexpr StenoStroke StenoJeffModifiers::IGNORE_MASK(
    StrokeMask::NUM | StrokeMask::SL | StrokeMask::TL | StrokeMask::KL |
    StrokeMask::PL | StrokeMask::WL | StrokeMask::HL | StrokeMask::RL |
    StrokeMask::A | StrokeMask::O | StrokeMask::STAR);

constexpr StenoStroke StenoJeffModifiers::CONTROL_MASK(StrokeMask::FR);
constexpr StenoStroke StenoJeffModifiers::SHIFT_MASK(StrokeMask::RR);
constexpr StenoStroke StenoJeffModifiers::SUPER_MASK(StrokeMask::PR);
constexpr StenoStroke StenoJeffModifiers::ALT_MASK(StrokeMask::BR);

//---------------------------------------------------------------------------

struct JeffModifiersData {
  uint32_t keyState : 24;
  uint8_t keyCode : 8;
};

const JeffModifiersData DATA[] = {
    {
        .keyState = StrokeMask::A,
        .keyCode = KeyCode::A,
    },
    {
        .keyState = StrokeMask::PL | StrokeMask::WL,
        .keyCode = KeyCode::B,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::RL,
        .keyCode = KeyCode::C,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::KL,
        .keyCode = KeyCode::D,
    },
    {
        .keyState = StrokeMask::E,
        .keyCode = KeyCode::E,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::PL,
        .keyCode = KeyCode::F,
    },
    {
        .keyState =
            StrokeMask::TL | StrokeMask::KL | StrokeMask::PL | StrokeMask::WL,
        .keyCode = KeyCode::G,
    },
    {
        .keyState = StrokeMask::HL,
        .keyCode = KeyCode::H,
    },
    {
        .keyState = StrokeMask::E | StrokeMask::U,
        .keyCode = KeyCode::I,
    },
    {
        .keyState =
            StrokeMask::SL | StrokeMask::KL | StrokeMask::WL | StrokeMask::RL,
        .keyCode = KeyCode::J,
    },
    {
        .keyState = StrokeMask::KL,
        .keyCode = KeyCode::K,
    },
    {
        .keyState = StrokeMask::HL | StrokeMask::RL,
        .keyCode = KeyCode::L,
    },
    {
        .keyState = StrokeMask::PL | StrokeMask::HL,
        .keyCode = KeyCode::M,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::PL | StrokeMask::HL,
        .keyCode = KeyCode::N,
    },
    {
        .keyState = StrokeMask::O,
        .keyCode = KeyCode::O,
    },
    {
        .keyState = StrokeMask::PL,
        .keyCode = KeyCode::P,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL,
        .keyCode = KeyCode::Q,
    },
    {
        .keyState = StrokeMask::RL,
        .keyCode = KeyCode::R,
    },
    {
        .keyState = StrokeMask::SL,
        .keyCode = KeyCode::S,
    },
    {
        .keyState = StrokeMask::TL,
        .keyCode = KeyCode::T,
    },
    {
        .keyState = StrokeMask::U,
        .keyCode = KeyCode::U,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::RL,
        .keyCode = KeyCode::V,
    },
    {
        .keyState = StrokeMask::WL,
        .keyCode = KeyCode::W,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::PL,
        .keyCode = KeyCode::X,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::RL,
        .keyCode = KeyCode::Y,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::TL | StrokeMask::KL |
                    StrokeMask::PL | StrokeMask::WL,
        .keyCode = KeyCode::Z,
    },

    // Navigation layer.
    {
        .keyState = StrokeMask::PL | StrokeMask::LR,
        .keyCode = KeyCode::UP,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::LR,
        .keyCode = KeyCode::LEFT,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::LR,
        .keyCode = KeyCode::DOWN,
    },
    {
        .keyState = StrokeMask::RL | StrokeMask::LR,
        .keyCode = KeyCode::RIGHT,
    },
    {
        .keyState =
            StrokeMask::TL | StrokeMask::PL | StrokeMask::HL | StrokeMask::LR,
        .keyCode = KeyCode::PAGE_UP,
    },
    {
        .keyState =
            StrokeMask::KL | StrokeMask::WL | StrokeMask::RL | StrokeMask::LR,
        .keyCode = KeyCode::PAGE_DOWN,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::KL | StrokeMask::LR,
        .keyCode = KeyCode::HOME,
    },
    {
        .keyState = StrokeMask::HL | StrokeMask::RL | StrokeMask::LR,
        .keyCode = KeyCode::END,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::LR,
        .keyCode = KeyCode::BACKSPACE,
    },
    {
        .keyState = StrokeMask::HL | StrokeMask::LR,
        .keyCode = KeyCode::DELETE,
    },

    // Keys layer.
    {
        .keyState =
            StrokeMask::TL | StrokeMask::KL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::ESC,
    },
    {
        .keyState =
            StrokeMask::HL | StrokeMask::RL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::TAB,
    },
    {
        .keyState = StrokeMask::HL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::APOSTROPHE,
    },
    {
        .keyState = StrokeMask::PL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::GRAVE,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::DOT,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::COMMA,
    },
    {
        .keyState =
            StrokeMask::KL | StrokeMask::WL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::SEMICOLON,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::PL | StrokeMask::KL |
                    StrokeMask::WL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::EQUAL,
    },
    {
        .keyState =
            StrokeMask::PL | StrokeMask::HL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::MINUS,
    },
    {
        .keyState =
            StrokeMask::WL | StrokeMask::RL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::SPACE,
    },
    {
        .keyState =
            StrokeMask::KL | StrokeMask::PL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::SLASH,
    },
    {
        .keyState =
            StrokeMask::TL | StrokeMask::WL | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::BACKSLASH,
    },
    {
        .keyState = StrokeMask::TL | StrokeMask::PL | StrokeMask::HL |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::L_BRACKET,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::RL |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::R_BRACKET,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::HL | StrokeMask::RL |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::ENTER,
    },

    // Number layer.
    {
        .keyState = StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_0,
    },
    {
        .keyState =
            StrokeMask::RL | StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_1,
    },
    {
        .keyState =
            StrokeMask::WL | StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_2,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::RL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_3,
    },
    {
        .keyState =
            StrokeMask::KL | StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_4,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::RL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_5,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_6,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::RL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_7,
    },
    {
        .keyState =
            StrokeMask::SL | StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_8,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::RL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::_9,
    },

    // Num pad numbers.
    {
        .keyState =
            StrokeMask::PL | StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_0,
    },
    {
        .keyState = StrokeMask::RL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_1,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_2,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::RL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_3,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_4,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::RL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_5,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_6,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::RL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::KP_7,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_8,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::RL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::KP_9,
    },

    // Function key layers.
    {
        .keyState = StrokeMask::RL | StrokeMask::TL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F1,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::TL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F2,
    },
    {
        .keyState = StrokeMask::WL | StrokeMask::RL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F3,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::TL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F4,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::RL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F5,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F6,
    },
    {
        .keyState = StrokeMask::KL | StrokeMask::WL | StrokeMask::RL |
                    StrokeMask::TL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F7,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::TL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F8,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::RL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F9,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::WL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F10,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::WL | StrokeMask::RL |
                    StrokeMask::TL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F11,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::KL | StrokeMask::TL |
                    StrokeMask::PL | StrokeMask::E | StrokeMask::U |
                    StrokeMask::LR,
        .keyCode = KeyCode::F12,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::KL | StrokeMask::RL |
                    StrokeMask::TL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F13,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::KL | StrokeMask::WL |
                    StrokeMask::TL | StrokeMask::PL | StrokeMask::E |
                    StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F14,
    },
    {
        .keyState = StrokeMask::SL | StrokeMask::KL | StrokeMask::WL |
                    StrokeMask::RL | StrokeMask::TL | StrokeMask::PL |
                    StrokeMask::E | StrokeMask::U | StrokeMask::LR,
        .keyCode = KeyCode::F15,
    },
};

//---------------------------------------------------------------------------

void StenoJeffModifiers::Process(const StenoKeyState &value,
                                 StenoAction action) {
  const StenoStroke stroke = value.ToStroke();

  bool isModifier = (stroke & TRIGGER_MASK) == TRIGGER_STROKE;

  switch (action) {
  case StenoAction::PRESS:
    if (isModifier) {
      if (!wasModifier) {
        wasModifier = true;
        nextProcessor.Process(value, StenoAction::CANCEL);
      }
      UpdateModifiers(stroke);
      return;
    } else if (wasModifier) {
      wasModifier = false;
      UpdateModifiers(StenoStroke());
    }
    break;

  case StenoAction::RELEASE:
    if (wasModifier) {
      if (value.IsEmpty()) {
        wasModifier = false;
      }
      UpdateModifiers(stroke);
      return;
    }
    break;

  case StenoAction::CANCEL:
    wasModifier = false;
    UpdateModifiers(StenoStroke());
    break;

  case StenoAction::TRIGGER:
    if (wasModifier || isModifier) {
      StenoStroke previousStroke = lastModifiers;
      UpdateModifiers(stroke);
      bool result = TriggerSendKey(stroke);
      UpdateModifiers(previousStroke);
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

void StenoJeffModifiers::UpdateModifiers(StenoStroke stroke) {
  lastModifiers = stroke;
  bool isControl = (stroke & CONTROL_MASK).IsNotEmpty();
  if (isControl != wasControl) {
    wasControl = isControl;
    if (isControl) {
      Key::Press(KeyCode::L_CTRL);
    } else {
      Key::Release(KeyCode::L_CTRL);
    }
  }

  bool isShift = (stroke & SHIFT_MASK).IsNotEmpty();
  if (isShift != wasShift) {
    wasShift = isShift;
    if (isShift) {
      Key::Press(KeyCode::L_SHIFT);
    } else {
      Key::Release(KeyCode::L_SHIFT);
    }
  }

  bool isSuper = (stroke & SUPER_MASK).IsNotEmpty();
  if (isSuper != wasSuper) {
    wasSuper = isSuper;
    if (isSuper) {
      Key::Press(KeyCode::L_META);
    } else {
      Key::Release(KeyCode::L_META);
    }
  }

  bool isAlt = (stroke & ALT_MASK).IsNotEmpty();
  if (isAlt != wasAlt) {
    wasAlt = isAlt;
    if (isAlt) {
      Key::Press(KeyCode::L_ALT);
    } else {
      Key::Release(KeyCode::L_ALT);
    }
  }
}

bool StenoJeffModifiers::TriggerSendKey(StenoStroke stroke) const {
  const uint32_t keyState = (stroke & DATA_MASK).GetKeyState();

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
  return (stroke & IGNORE_MASK).IsEmpty();
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
