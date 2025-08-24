//---------------------------------------------------------------------------
// Implements passport protocol:
// https://www.eclipsecat.com/sites/default/files/Passport%20protocol_0.pdf
//---------------------------------------------------------------------------

#include "passport.h"
#include "../clock.h"
#include "../console.h"
#include "../hal/serial_port.h"
#include "../writer.h"

//---------------------------------------------------------------------------

constexpr uint8_t PASSPORT_LOOKUP[] = {
    'S', // S1
    'C', // S2
    'T', // TL
    'K', // KL
    'P', // PL
    'W', // WL
    'H', // HL
    'R', // RL
    'A', // A
    'O', // O
    '~', // STAR1
    '~', // STAR2
    '*', // STAR3
    '*', // STAR4
    'E', // E
    'U', // U
    'F', // FR
    'Q', // RR
    'N', // PR
    'B', // BR
    'L', // LR
    'G', // GR
    'Y', // TR
    'X', // SR
    'D', // DR
    'Z', // ZR
    '#', // NUM1
    '#', // NUM2
    '#', // NUM3
    '#', // NUM4
    '#', // NUM5
    '#', // NUM6
    '#', // NUM7
    '#', // NUM8
    '#', // NUM9
    '#', // NUM10
    '#', // NUM11
    '#', // NUM12
    '^', // FUNCTION, X1, ^
    '+', // POWER, X2, +
    '!', // RES1, X3
    0,   // RES2, X4
    0,   // X5
    0,   // X6
    0,   // X7
    0,   // X8
    0,   // X9
    0,   // X10
    0,   // X11
    0,   // X12
    0,   // X13
    0,   // X14
    0,   // X15
    0,   // X16
    0,   // X17
    0,   // X18
    0,   // X19
    0,   // X20
    0,   // X21
    0,   // X22
    0,   // X23
    0,   // X24
    0,   // X25
    0,   // X26
};
static_assert(sizeof(PASSPORT_LOOKUP) == (int)StenoKey::COUNT,
              "Passport table must be complete");

//---------------------------------------------------------------------------

void StenoPassport::Process(const StenoKeyState &value, StenoAction action) {
  next->Process(value, action);
  if (action != StenoAction::TRIGGER) {
    return;
  }

  BufferWriter buffer;
  buffer.Printf("<%zu/", counter++);

  char strokes[64];
  char *p = strokes;

  uint64_t localKeyState = value.GetRawKeyState();
  uint8_t lastLetter = 0;
  while (localKeyState) {
    const int index = __builtin_ctzll(localKeyState);
    const uint8_t letter = PASSPORT_LOOKUP[index];
    if (letter && letter != lastLetter) {
      *p++ = letter;

      // Stroke weight, from 0-f
      *p++ = 'f';

      lastLetter = letter;
    }

    localKeyState &= localKeyState - 1;
  }

  *p = '\0';

  buffer.Printf("%s/%u>", strokes, Clock::GetMilliseconds());

  SerialPort::SendData(buffer.GetBuffer(), buffer.GetCount());
}

void StenoPassport::PrintInfo() const {
  Console::Printf("  Passport: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------
