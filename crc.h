//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

uint8_t Crc8(const void *p, size_t count);
uint32_t Crc16Ccitt(const void *p, size_t count);
uint32_t Crc32(const void *p, size_t count);
consteval uint32_t EmptyCrc32() { return 0; }

//---------------------------------------------------------------------------
