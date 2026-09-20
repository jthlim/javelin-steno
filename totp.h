//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class IWriter;

//---------------------------------------------------------------------------

enum class TotpAlgorithmId : uint8_t {
  SHA1,
  SHA256,
  SHA512,
};

struct Totp {
  uint32_t magic;
  TotpAlgorithmId algorithmId;
  uint8_t digitCount;
  uint8_t tokenPeriod;
  uint8_t secretLength;
  uint8_t secret[0];

  static const uint32_t TOTP_MAGIC = 0x304f544a; // 'JTO0'

  bool IsValid() const { return magic == TOTP_MAGIC; }

  static void Generate(IWriter &output, const void *data);
  static int GetPeriod(const void *data);

  void GenerateDecrypted(IWriter &output) const;
};

//---------------------------------------------------------------------------