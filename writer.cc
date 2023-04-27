//---------------------------------------------------------------------------

#include "writer.h"
#include "console.h"
#include "str.h"
#include <string.h>

//---------------------------------------------------------------------------

const int FLAG_FILL_ZERO = 1;
const int FLAG_LENGTH_64_BIT = 2;

//---------------------------------------------------------------------------

IWriter::ClassData IWriter::classData = {
    {nullptr, nullptr, nullptr, nullptr},
    0,
    &ConsoleWriter::instance,
};

void IWriter::Push(IWriter *writer) {
  classData.data[classData.count++] = classData.active;
  classData.active = writer;
}

void IWriter::Pop() {
  if (classData.count == 0) {
    return;
  }
  classData.active = classData.data[--classData.count];
}

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

void IWriter::Vprintf(const char *p, va_list args) {
  const char *spanStart = p;
  char scratch[32];

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
    int fill = ' ';
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

    case '\0': // Shouldn't happen, but catc
    default:
      // Just use span start.
      continue;
    }
    WriteSegment(flags, start, end, width);
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

ConsoleWriter ConsoleWriter::instance;

#ifdef RUN_TESTS

std::vector<char> Console::history;

__attribute__((weak)) void ConsoleWriter::Write(const char *data,
                                                size_t length) {
  std::copy(data, data + length, std::back_inserter(Console::history));
}

#endif

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
