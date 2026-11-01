//---------------------------------------------------------------------------

#include "bit_field.h"
#include "container/list.h"
#include "unit_test.h"

//---------------------------------------------------------------------------

TEST_BEGIN("BitField: EmptySet iterates properly") {
  List<size_t> setBits;
  BitField<256> bitfield = {};
  for (const size_t i : bitfield) {
    setBits.Add(i);
  }
  assert(setBits.GetCount() == 0);
}
TEST_END

TEST_BEGIN("BitField: Set bits iterate properly") {
  List<size_t> setBits;
  BitField<256> bitfield = {};
  bitfield.Set(1);
  bitfield.Set(252);
  for (const size_t i : bitfield) {
    setBits.Add(i);
  }
  assert(setBits.GetCount() == 2);
  assert(setBits[0] == 1);
  assert(setBits[1] == 252);

  assert(bitfield.GetFirstBitIndex() == 1);
  assert(bitfield.GetLastBitIndex() == 252);
}
TEST_END

TEST_BEGIN("BitField: Test GetFirstBitIndex and GetLastBitIndex") {
  BitField<128> bitfield = {};
  for (char c : "aeiou") {
    if (c) {
      bitfield.Set(c);
    }
  }
  assert(bitfield.GetFirstBitIndex() == 'a');
  assert(bitfield.GetLastBitIndex() == 'u');
}
TEST_END

TEST_BEGIN("BitField: Test GetRange returns expected values") {
  BitField<128> bitfield = {};
  for (size_t i = 0; i < 128; i += 2) {
    bitfield.Set(i);
  }

  assert(bitfield.GetRange(0, 8) == 0x55);
  assert(bitfield.GetRange(1, 9) == 0xaa);
  assert(bitfield.GetRange(0, 32) == 0x55555555);
  assert(bitfield.GetRange(56, 64) == 0x55);
}
TEST_END

//---------------------------------------------------------------------------
