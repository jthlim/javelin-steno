//---------------------------------------------------------------------------

#include "writer.h"
#include "console.h"
#include "str.h"
#include "stroke.h"
#include <string.h>

//---------------------------------------------------------------------------

const int FLAG_FILL_ZERO = 1;
const int FLAG_LENGTH_64_BIT = 2;

//---------------------------------------------------------------------------

static void Reverse(char *start, char *end) {
  for (--end; start < end; ++start, --end) {
    char t = *start;
    *start = *end;
    *end = t;
  }
}

static char *WriteReversedUint32(char *p, uint32_t v) {
  do {
    *p++ = v % 10 + '0';
    v /= 10;
  } while (v);
  return p;
}

static char *WriteReversedUint64(char *p, uint64_t v) {
  do {
    *p++ = v % 10 + '0';
    v /= 10;
  } while (v);
  return p;
}

static char *WriteReversedHex32(char *p, uint32_t v, const char *alphabet) {
  do {
    *p++ = alphabet[v & 0xf];
    v >>= 4;
  } while (v);
  return p;
}

static char *WriteReversedHex64(char *p, uint64_t v, const char *alphabet) {
  do {
    *p++ = alphabet[v & 0xf];
    v >>= 4;
  } while (v);
  return p;
}

void IWriter::Printf(const char *p, ...) {
  va_list args;
  va_start(args, p);
  Vprintf(p, args);
  va_end(args);
}

void IWriter::Vprintf(const char *p, va_list args) {
  const char *spanStart = p;
  char scratch[64];

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
        end = WriteReversedUint64(t, v);
      } else {
        int32_t v = va_arg(args, int32_t);
        if (v < 0) {
          *t++ = '-';
          v = -v;
        }
        end = WriteReversedUint32(t, v);
      }
      Reverse(t, end);
      break;
    }

    case 'u':
      if (flags & FLAG_LENGTH_64_BIT) {
        uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedUint64(start, v);
      } else {
        uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedUint32(start, v);
      }
      Reverse(start, end);
      break;

    case 'x':
      if (flags & FLAG_LENGTH_64_BIT) {
        uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex64(start, v, "0123456789abcdef");
      } else {
        uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex32(start, v, "0123456789abcdef");
      }
      Reverse(start, end);
      break;

    case 'X':
      if (flags & FLAG_LENGTH_64_BIT) {
        uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex64(start, v, "0123456789ABCDEF");
      } else {
        uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex32(start, v, "0123456789ABCDEF");
      }
      Reverse(start, end);
      break;

    case 'p':
      if (sizeof(void *) == sizeof(uint64_t)) {
        uint64_t v = va_arg(args, uint64_t);
        end = WriteReversedHex64(start, v, "0123456789abcdef");
      } else {
        uint32_t v = va_arg(args, uint32_t);
        end = WriteReversedHex32(start, v, "0123456789abcdef");
      }
      Reverse(start, end);
      break;

    case 's':
      start = va_arg(args, char *);
      end = start + Str::Length(start);
      break;

    case '%':
      spanStart = p;
      ++p;
      continue;

    case 'B': {
      // Write the string true or false based on a bool.
      int value = va_arg(args, int);
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
      char *p = stroke->ToString(scratch);
      Write(scratch, p - scratch);
      goto NextSegment;
    }

    case 'T': {
      // Write multiple strokes.
      const StenoStroke *strokes = va_arg(args, const StenoStroke *);
      size_t strokeCount = va_arg(args, size_t);
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
    case 'J': {
      // Write as JSON
      const char *p = va_arg(args, char *);
      size_t length = Str::Length(p);
      char *jsonBuffer =
          length <= sizeof(scratch) / 2 ? scratch : (char *)malloc(2 * length);
      char *end = Str::WriteJson(jsonBuffer, p);
      WriteSegment(flags, jsonBuffer, end, width);
      if (jsonBuffer != scratch) {
        free(jsonBuffer);
      }
      goto NextSegment;
    }

    case '\0': // Shouldn't happen, but catc
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

static const char SPACES[] = "                ";
static const char ZEROS[] = "0000000000000000";

void IWriter::WriteSegment(int flags, char *start, char *end, int width) {
  size_t length = end - start;
  if (length < width) {
    size_t fillCount = width - length;
    while (fillCount) {
      size_t fillSegmentCount = fillCount > 16 ? 16 : fillCount;

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
    : bufferUsedCount(0), bufferSize(128), buffer((char *)malloc(128)) {}

void BufferWriter::Write(const char *data, size_t length) {
  size_t newUsedCount = bufferUsedCount + length;
  if (newUsedCount > bufferSize) {
    do {
      bufferSize *= 2;
    } while (bufferSize < newUsedCount);

    buffer = (char *)realloc(buffer, bufferSize);
  }
  memcpy(buffer + bufferUsedCount, data, length);
  bufferUsedCount += length;
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
