//---------------------------------------------------------------------------
//
// Format strings:
// * %B     - bool
// * %c     - char
// * %C     - Unicode (uint32_t)
// * %d, %i - int
// * %b     - const void*, size_t count - Yaml Data in in base64
// * %D     - const void*, size_t count - Data in in base64
// * %J     - char* in escaped JSON
// * %p     - void*
// * %s     - const char*
// * %t     - StenoStroke*
// * %T     - StenoStroke*, size_t count -- always quoted
// * %O     - StenoStroke*, size_t count -- yaml
// * %u     - uint32_t
// * %x     - Hex
// * %X     - Upper case hex
// * %Y     - YAML string
// * %Z     - Writes a zero (nul) byte. This takes no parameters.
//
//---------------------------------------------------------------------------

#include "writer.h"
#include "clamp.h"
#include "str.h"
#include "stroke.h"
#include "unicode.h"
#include "utf8_pointer.h"
#include <string.h>

//---------------------------------------------------------------------------

constexpr int FLAG_FILL_ZERO = 1;
constexpr int FLAG_LENGTH_64_BIT = 2;

constexpr char BASE64_ALPHABET[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//---------------------------------------------------------------------------

static void Reverse(char *start, char *end) {
  for (--end; start < end; ++start, --end) {
    const char t = *start;
    *start = *end;
    *end = t;
  }
}

template <typename T> static char *WriteReversed(char *p, T v) {
  do {
    *p++ = v % 10 + '0';
    v /= 10;
  } while (v);
  return p;
}

template <typename T>
static char *WriteReversedHex(char *p, T v, const char *alphabet) {
  do {
    *p++ = alphabet[v & 0xf];
    v >>= 4;
  } while (v);
  return p;
}

[[gnu::noinline]] void IWriter::Dump(const void *data, size_t length) {
  const uint8_t *p = (const uint8_t *)data;

  // clang-format off
  // 0         1         2         3         4         5         6         7        7
  // 0         0         0         0         0         0         0         0        9
  // 00000000:  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|↵
  // clang-format on
  char line[80];
  line[78] = '|';
  line[79] = '\n';

  for (size_t i = 0; i < length; ++i) {
    const size_t pos = i & 15;
    if (pos == 0) {
      if (i != 0) {
        Write(line, 80);
      }
      Str::Sprintf(line, "%08x:", i);
      memset(line + 9, ' ', 78 - 9);
      line[61] = '|';
    }
    const size_t offset = 11 + 3 * pos + (pos >= 8);
    const uint8_t c = p[i];
    line[offset] = "0123456789ABCDEF"[c >> 4];
    line[offset + 1] = "0123456789ABCDEF"[c & 0xf];
    line[62 + pos] = 32 <= c && c < 128 ? c : '.';
  }

  Write(line, 80);
}

void IWriter::WriteString(const char *s) { Write(s, Str::Length(s)); }

void IWriter::WriteBase64(const void *data, size_t length) {
  // 3*8 bits -> 4*6
  const uint8_t *p = (const uint8_t *)data;
  while (length >= 3) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
    // There's some XIP reading bug that this sequence avoids.
    // Using the default code causes the bzip2 header to be corrupted.
    const uint32_t value = __builtin_bswap32(*(uint32_t *)p << 8);
    p += 3;
#else
    const uint32_t value = (p[0] << 16) + (p[1] << 8) + p[2];
    p += 3;
#endif

    WriteByte(BASE64_ALPHABET[value >> 18]);
    WriteByte(BASE64_ALPHABET[(value >> 12) & 0x3f]);
    WriteByte(BASE64_ALPHABET[(value >> 6) & 0x3f]);
    WriteByte(BASE64_ALPHABET[value & 0x3f]);

    length -= 3;
  }

  switch (length) {
  case 1: {
    const uint32_t value = p[0];
    WriteByte(BASE64_ALPHABET[value >> 2]);
    WriteByte(BASE64_ALPHABET[(value << 4) & 0x3f]);
    WriteByte('=');
    WriteByte('=');
  } break;

  case 2: {
    const uint32_t value = (p[0] << 8) + p[1];
    WriteByte(BASE64_ALPHABET[value >> 10]);
    WriteByte(BASE64_ALPHABET[(value >> 4) & 0x3f]);
    WriteByte(BASE64_ALPHABET[(value << 2) & 0x3f]);
    WriteByte('=');
  } break;
  }
}

void IWriter::WriteIntList(const int32_t *data, size_t count) {
  int i = 0;
  while (i < count) {
    int j = i;
    while (j < count && data[j] == 0) {
      ++j;
    }
    WriteVarUint(j - i);
    i = j;

    while (j < count && data[j] != 0) {
      ++j;
    }
    WriteVarUint(j - i);
    for (; i < j; ++i) {
      WriteVarInt(data[i]);
    }
  }
}

void IWriter::WriteVarUint(uint32_t x) {
  if (x < 0x80) {
    WriteByte(x << 1);
  } else if (x < 0x4000) {
    const uint16_t buffer = (x << 2) | 1;
    Write((const char *)&buffer, 2);
  } else if (x < 0x200000) {
    const uint32_t buffer = (x << 3) | 3;
    Write((const char *)&buffer, 3);
  } else if (x < 0x10000000) {
    const uint32_t buffer = (x << 4) | 7;
    Write((const char *)&buffer, 4);
  } else {
    WriteByte((x << 5) | 15);
    const uint32_t buffer = x >> 3;
    Write((const char *)&buffer, 4);
  }
}

void IWriter::Printf(const char *p, ...) {
  va_list args;
  va_start(args, p);
  Vprintf(p, args);
  va_end(args);
}

bool IWriter::IsNumber(const char *p) {
  // ^[+-]\d*(.\d*)?(e[+-]?\d+)$
  if (*p == '-' || *p == '+') {
    ++p;
  }

  bool hasDigit = false;
  while (Unicode::IsAsciiDigit(*p)) {
    ++p;
    hasDigit = true;
  }

  if (*p == '.') {
    ++p;
    while (Unicode::IsAsciiDigit(*p)) {
      ++p;
      hasDigit = true;
    }
  }

  if (!hasDigit) {
    return false;
  }

  if (*p == 'e' || *p == 'E') {
    ++p;

    if (*p == '-' || *p == '+') {
      ++p;
    }

    if (!Unicode::IsAsciiDigit(*p++)) {
      return false;
    }

    while (Unicode::IsAsciiDigit(*p)) {
      ++p;
    }
  }

  return *p == '\0';
}

bool IWriter::IsYamlSafe(const char *p) {
  switch (*p) {
  case 0 ... ' ':
  case '*':
  case '!':
  case '@':
  case '%':
  case '&':
  case '>':
  case '\'':
  case '\"':
  case '`':
  case '|':
    return false;

  case '~':
  case '?':
    if (p[1] <= ' ') {
      return false;
    }
    break;

  case 'f':
    if (Str::Eq(p, "false")) {
      return false;
    }
    break;

  case 'n':
    if (Str::Eq(p, "null")) {
      return false;
    }
    break;

  case 't':
    if (Str::Eq(p, "true")) {
      return false;
    }
    break;

  case '-':
    if (p[1] <= ' ') {
      return false;
    }
    [[fallthrough]];

  case '.':
  case '+':
  case '0' ... '9':
    if (IsNumber(p)) {
      return false;
    }
    break;
  }

  for (;;) {
    const int c = *p++;
    switch (c) {
    case '\0':
      switch (p[-2]) {
      case 0 ... ' ':
        return false;
      default:
        return true;
      }

    case ':':
      if (*p <= ' ') {
        return false;
      }
      break;

    case 1 ... 0x1f:
    case '#':
    case ',':
    case '[':
    case ']':
    case '{':
    case '}':
      return false;
    }
  }
}

bool IsInRange(uint32_t c, uint32_t lower, uint32_t upper) {
  return lower <= c && c < upper;
}

bool IWriter::IsYamlSafeData(const uint8_t *p, size_t length) {
  if (length == 0) {
    return false;
  }

  if (length % 3 == 0) {
    return true;
  }

  if (length == 3) {
    // Encodes to 'true'
    if (p[0] == 0xb6 && p[1] == 0xbb && p[2] == 0x9e) {
      return false;
    }

    // Encodes to 'null'
    if (p[0] == 0x9e && p[1] == 0xe9 && p[2] == 0x65) {
      return false;
    }
  }

  const uint32_t lowerDigitValue = 52;
  const uint32_t upperDigitValue = 62;
  const uint32_t upperDigitAndPlusValue = 63; // Start of with digits and plus.

#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  // There's some XIP reading bug that this sequence avoids.
  // Using the default code causes the bzip2 header to be corrupted.
  const uint32_t value = __builtin_bswap32(*(uint32_t *)p << 8);
  p += 3;
#else
  const uint32_t value = (p[0] << 16) + (p[1] << 8) + p[2];
  p += 3;
#endif

  if (!IsInRange(value >> 18, lowerDigitValue, upperDigitAndPlusValue)) {
    return true;
  }
  if (!IsInRange((value >> 12) & 0x3f, lowerDigitValue, upperDigitValue)) {
    return true;
  }
  if (!IsInRange((value >> 6) & 0x3f, lowerDigitValue, upperDigitValue)) {
    return true;
  }
  if (!IsInRange(value & 0x3f, lowerDigitValue, upperDigitValue)) {
    return true;
  }
  length -= 3;

  while (length >= 3) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
    // There's some XIP reading bug that this sequence avoids.
    // Using the default code causes the bzip2 header to be corrupted.
    const uint32_t value = __builtin_bswap32(*(uint32_t *)p << 8);
    p += 3;
#else
    const uint32_t value = (p[0] << 16) + (p[1] << 8) + p[2];
    p += 3;
#endif

    if (!IsInRange(value >> 18, lowerDigitValue, upperDigitValue)) {
      return true;
    }
    if (!IsInRange((value >> 12) & 0x3f, lowerDigitValue, upperDigitValue)) {
      return true;
    }
    if (!IsInRange((value >> 6) & 0x3f, lowerDigitValue, upperDigitValue)) {
      return true;
    }
    if (!IsInRange(value & 0x3f, lowerDigitValue, upperDigitValue)) {
      return true;
    }
    length -= 3;
  }

  return false;
}

// This is forced not-inline to avoid stack depth on recursive calls.
[[gnu::noinline]]
void IWriter::AddStroke(StenoStroke stroke) {
  char strokeBuffer[StenoStroke::MAX_STRING_LENGTH];
  char *p = stroke.ToString(strokeBuffer);
  Write(strokeBuffer, p - strokeBuffer);
}

void IWriter::Vprintf(const char *p, va_list args) {
  const char *spanStart = p;
  char scratch[128];

  static_assert(sizeof(scratch) >= StenoStroke::MAX_STRING_LENGTH + 1);

  for (;;) {
    for (;;) {
      if (*p == '\0') {
        if (p != spanStart) {
          Write(spanStart, p - spanStart);
        }
        return;
      }
      if (*p == '%') {
        break;
      }
      ++p;
    }
    if (p != spanStart) {
      Write(spanStart, p - spanStart);
    }
    spanStart = p;

    int flags = 0;
    int width = 0;
    ++p;
    if (*p == '0') {
      flags |= FLAG_FILL_ZERO;
      ++p;
    } else if (*p == ' ') {
      ++p;
    }
    while ('0' <= *p && *p <= '9') {
      width = 10 * width + *p++ - '0';
    }
    if (*p == 'l') {
      ++p;
      if (*p == 'l') {
        flags |= FLAG_LENGTH_64_BIT;
        ++p;
      }
    } else if (*p == 'h' || *p == 'z') {
      ++p;
    } else if (*p == 'z') {
      if (sizeof(size_t) == sizeof(uint64_t)) {
        flags |= FLAG_LENGTH_64_BIT;
      }
      ++p;
    }

    char *start = scratch;
    char *end;
    switch (*p) {
    case 'd':
    case 'i': {
      char *t = start;
      if (flags & FLAG_LENGTH_64_BIT) {
        int64_t v = va_arg(args, int64_t);
        if (v < 0) {
          *t++ = '-';
          v = -v;
        }
        end = WriteReversed<uint64_t>(t, v);
      } else {
        int32_t v = va_arg(args, int32_t);
        if (v < 0) {
          *t++ = '-';
          v = -v;
        }
        end = WriteReversed<uint32_t>(t, v);
      }
      Reverse(t, end);
      break;
    }

    case 'u':
      if (flags & FLAG_LENGTH_64_BIT) {
        const uint64_t v = va_arg(args, uint64_t);
        end = WriteReversed<uint64_t>(start, v);
      } else {
        const uint32_t v = va_arg(args, uint32_t);
        end = WriteReversed<uint32_t>(start, v);
      }
      Reverse(start, end);
      break;

    case 'x':
      if (flags & FLAG_LENGTH_64_BIT) {
        const uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex<uint64_t>(start, v, "0123456789abcdef");
      } else {
        const uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex<uint32_t>(start, v, "0123456789abcdef");
      }
      Reverse(start, end);
      break;

    case 'X':
      if (flags & FLAG_LENGTH_64_BIT) {
        const uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex<uint64_t>(start, v, "0123456789ABCDEF");
      } else {
        const uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex<uint32_t>(start, v, "0123456789ABCDEF");
      }
      Reverse(start, end);
      break;

    case 'p':
      if constexpr (sizeof(void *) == sizeof(uint64_t)) {
        const uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex<uint64_t>(start, v, "0123456789abcdef");
      } else {
        const uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex<uint32_t>(start, v, "0123456789abcdef");
      }
      Reverse(start, end);
      break;

    case 'c':
      *start = va_arg(args, int);
      end = start + 1;
      break;

    case 'C': {
      const uint32_t c = va_arg(args, int);
      Utf8Pointer p(start);
      p.SetAndAdvance(c);
      end = p.GetRawPointer();
    } break;

    case 's':
      start = va_arg(args, char *) + printfPointerOffset;
      end = start + Str::Length(start);
      break;

    case '%':
      spanStart = p;
      ++p;
      continue;

    case 'B': {
      // Write the string true or false based on a bool.
      const int value = va_arg(args, int);
      if (value) {
        Write("true", 4);
      } else {
        Write("false", 5);
      }
      goto NextSegment;
    }

    case 't': {
      // Write single stroke.
      const StenoStroke *stroke = va_arg(args, const StenoStroke *);
      stroke = (const StenoStroke *)(intptr_t(stroke) + printfPointerOffset);
      char *p = stroke->ToString(scratch + 1);
      *p = '\0';
      if (IsYamlSafe(scratch + 1)) {
        Write(scratch + 1, p - (scratch + 1));
      } else {
        scratch[0] = '\"';
        *p++ = '\"';
        Write(scratch, p - scratch);
      }
      goto NextSegment;
    }

    case 'O': {
      // Write multiple strokes.
      const StenoStroke *strokes = va_arg(args, const StenoStroke *);
      strokes = (const StenoStroke *)(intptr_t(strokes) + printfPointerOffset);
      const size_t strokeCount = va_arg(args, size_t);

      BufferWriter buffer(scratch, sizeof(scratch));
      buffer.WriteByte('\"');
      for (size_t j = 0; j < strokeCount; ++j) {
        if (j != 0) {
          buffer.WriteByte('/');
        }
        buffer.AddStroke(strokes[j]);
      }
      buffer.WriteByte('\0');
      const size_t length = buffer.GetCount();
      const char *outline = buffer.GetBuffer();

      if (IsYamlSafe(outline + 1)) {
        Write(outline + 1, length - 1);
      } else {
        buffer.RemoveByte();
        buffer.WriteByte('\"');
        Write(outline, length);
      }
      goto NextSegment;
    }

    case 'T': {
      // Write multiple strokes.
      const StenoStroke *strokes = va_arg(args, const StenoStroke *);
      strokes = (const StenoStroke *)(intptr_t(strokes) + printfPointerOffset);
      const size_t strokeCount = va_arg(args, size_t);
      for (size_t j = 0; j < strokeCount; ++j) {
        char *p = scratch;
        if (j != 0) {
          *p++ = '/';
        }
        p = strokes[j].ToString(p);
        Write(scratch, p - scratch);
      }
      goto NextSegment;
    }
    case 'b': {
      // Write Data (void*, length) as Base64
      const uint8_t *data = va_arg(args, const uint8_t *);
      data += printfPointerOffset;
      const size_t length = va_arg(args, size_t);
      if (IsYamlSafeData(data, length)) {
        WriteBase64(data, length);
      } else {
        WriteByte('\"');
        WriteBase64(data, length);
        WriteByte('\"');
      }
      goto NextSegment;
    }
    case 'D': {
      // Write Data (void*, length) as Base64
      const uint8_t *data = va_arg(args, const uint8_t *);
      data += printfPointerOffset;
      const size_t length = va_arg(args, size_t);
      WriteBase64(data, length);
      goto NextSegment;
    }
    case 'Y': {
      // Write as YAML
      char *p = va_arg(args, char *) + printfPointerOffset;
      const size_t length = Str::Length(p);
      if (IsYamlSafe(p)) {
        start = p;
        end = p + length;
        break;
      }

      size_t maxBufferSizeRequired = 2 * length + 2;
      char *jsonBuffer = sizeof(scratch) >= maxBufferSizeRequired
                             ? scratch
                             : (char *)malloc(maxBufferSizeRequired);
      jsonBuffer[0] = '\"';
      char *end = Str::WriteJson(jsonBuffer + 1, p);
      *end++ = '\"';
      WriteSegment(flags, jsonBuffer, end, width);
      if (jsonBuffer != scratch) {
        free(jsonBuffer);
      }
      goto NextSegment;
    }
    case 'J': {
      // Write as JSON
      const char *p = va_arg(args, char *) + printfPointerOffset;
      const size_t length = Str::Length(p);
      char *jsonBuffer =
          length <= sizeof(scratch) / 2 ? scratch : (char *)malloc(2 * length);
      char *end = Str::WriteJson(jsonBuffer, p);
      WriteSegment(flags, jsonBuffer, end, width);
      if (jsonBuffer != scratch) {
        free(jsonBuffer);
      }
      goto NextSegment;
    }

    case 'Z':
      WriteByte('\0');
      goto NextSegment;

    case '\0': // Shouldn't happen
    default:
      // Just use span start.
      continue;
    }
    WriteSegment(flags, start, end, width);

  NextSegment:
    ++p;
    spanStart = p;
  }
}

static constexpr char SPACES[] = "                ";
static constexpr char ZEROS[] = "0000000000000000";

void IWriter::WriteSegment(int flags, const char *start, const char *end,
                           int width) {
  const size_t length = end - start;
  if (length < width) {
    size_t fillCount = width - length;
    while (fillCount) {
      const size_t fillSegmentCount = ClampMax(fillCount, 16);

      const char *fill = (flags & FLAG_FILL_ZERO) ? ZEROS : SPACES;
      Write(fill, fillSegmentCount);

      fillCount -= fillSegmentCount;
    }
  }
  Write(start, length);
}

//---------------------------------------------------------------------------

NullWriter NullWriter::instance;

void NullWriter::WriteByte(char c) {}
void NullWriter::Write(const char *data, size_t length) {}

//---------------------------------------------------------------------------

void CountWriter::WriteByte(char c) { ++count; }
void CountWriter::Write(const char *data, size_t length) { count += length; }

void MemoryWriter::WriteByte(char c) { *target++ = c; }

void MemoryWriter::Write(const char *data, size_t length) {
  memcpy(target, data, length);
  target += length;
}

//---------------------------------------------------------------------------

BufferWriter::BufferWriter()
    : bufferUsedCount(0), bufferSize(128), buffer((char *)malloc(128)),
      inlineBuffer(nullptr) {}

void BufferWriter::Write(const char *data, size_t length) {
  const size_t newUsedCount = bufferUsedCount + length;
  if (newUsedCount > bufferSize) {
    do {
      bufferSize *= 2;
    } while (bufferSize < newUsedCount);

    Reallocate();
  }
  memcpy(buffer + bufferUsedCount, data, length);
  bufferUsedCount += length;
}

void BufferWriter::WriteByte(char c) {
  if (bufferUsedCount >= bufferSize) {
    bufferSize *= 2;
    Reallocate();
  }
  buffer[bufferUsedCount++] = c;
}

void BufferWriter::Reallocate() {
  if (buffer == inlineBuffer) {
    buffer = (char *)malloc(bufferSize);
    memcpy(buffer, inlineBuffer, bufferUsedCount);
  } else {
    buffer = (char *)realloc(buffer, bufferSize);
  }
}

//---------------------------------------------------------------------------

void BlockWriterBase::WriteByte(char c) {
  buffer[used++] = c;
  if (used == size) {
    Flush(buffer, used);
    used = 0;
  }
}

void BlockWriterBase::Write(const char *data, size_t length) {
  if (used) {
    const size_t remaining = size - used;
    if (length < remaining) {
      memcpy(buffer + used, data, length);
      used += length;
      return;
    }
    memcpy(buffer + used, data, remaining);
    data += remaining;
    length -= remaining;
    Flush(buffer, size);
  }

  while (length >= size) {
    Flush(data, size);
    data += size;
    length -= size;
  }

  used = length;
  memcpy(buffer, data, length);
}

void BlockWriterBase::Flush() {
  if (used) {
    Flush(buffer, used);
    used = 0;
  }
}

//---------------------------------------------------------------------------

void Base64Writer::Flush(const char *data, size_t length) {
  next->WriteBase64(data, length);
}

//---------------------------------------------------------------------------

void LimitedBufferWriter::Write(const char *data, size_t length) {
  if (bufferUsedCount + length > BUFFER_SIZE) {
    length = BUFFER_SIZE - bufferUsedCount;
  }
  memcpy(buffer + bufferUsedCount, data, length);
  bufferUsedCount += length;
}

void LimitedBufferWriter::AddTrailingNull() {
  while (bufferUsedCount > 0 && buffer[bufferUsedCount - 1] == '\n') {
    --bufferUsedCount;
  }
  buffer[bufferUsedCount] = '\0';
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("IWriter::IsNumber returns expected results") {
  assert(IWriter::IsNumber("1"));
  assert(IWriter::IsNumber("+1"));
  assert(IWriter::IsNumber("-1"));
  assert(IWriter::IsNumber("1."));
  assert(IWriter::IsNumber("1.2"));
  assert(IWriter::IsNumber("1.23"));
  assert(IWriter::IsNumber(".23"));
  assert(IWriter::IsNumber("1e1"));
  assert(IWriter::IsNumber("1e+1"));
  assert(IWriter::IsNumber("1e-12"));
  assert(IWriter::IsNumber("1e-123"));

  assert(!IWriter::IsNumber(""));
  assert(!IWriter::IsNumber("+"));
  assert(!IWriter::IsNumber("-"));
  assert(!IWriter::IsNumber("."));
  assert(!IWriter::IsNumber("1e"));
  assert(!IWriter::IsNumber("1.23e"));
  assert(!IWriter::IsNumber("1e-123.0"));
}
TEST_END

TEST_BEGIN("IWriter::IsYamlSafe returns expected results") {
  assert(IWriter::IsYamlSafe("34f"));
  assert(IWriter::IsYamlSafe("-to"));

  assert(!IWriter::IsYamlSafe("34"));
  assert(!IWriter::IsYamlSafe("true"));
  assert(!IWriter::IsYamlSafe("false"));
  assert(!IWriter::IsYamlSafe("null"));
}
TEST_END

//---------------------------------------------------------------------------
