//---------------------------------------------------------------------------

#include "hash.h"

//---------------------------------------------------------------------------

inline uint32_t RotateLeft(uint32_t v, int shift) {
  return (v << shift) | (v >> (32 - shift));
}

//---------------------------------------------------------------------------

[[gnu::noinline, gnu::weak]] uint32_t JavelinHash::Hash(const uint32_t *data,
                                                        size_t wordCount) {
  uint32_t s0 = prime0;
  uint32_t s1 = prime0 + prime1;

  while (wordCount >= 2) {
    s0 = RotateLeft(s0 + data[0] * prime0, 13) * prime1;
    s1 = RotateLeft(s1 + data[1] * prime0, 13) * prime1;
    data += 2;
    wordCount -= 2;
  }
  if (wordCount) {
    s0 = RotateLeft(s0 + data[0] * prime0, 13) * prime1;
  }
  return s0 ^ s1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("JavelinHash: Test hash results") {
  assert(JavelinHash::Hash(nullptr, 0) == 0xba143d99);

  const uint32_t data0[4] = {};
  assert(JavelinHash::Hash(data0, 1) == 0x7fed0922);
  assert(JavelinHash::Hash(data0, 4) == 0x1a9f20e1);

  const uint32_t data1[4] = {0, 0, 0, 1};
  assert(JavelinHash::Hash(data1, 4) == 0x3b254860);
}
TEST_END

//---------------------------------------------------------------------------