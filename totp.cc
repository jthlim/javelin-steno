//---------------------------------------------------------------------------

#include "totp.h"
#include "writer.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void Totp::Generate(IWriter &output, const void *data) {}
[[gnu::weak]] int Totp::GetPeriod(const void *data) { return 0; }

//---------------------------------------------------------------------------