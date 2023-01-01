//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct StenoKeyPressToken {
  enum class Type {
    KEY,
    OPEN_PAREN,
    CLOSE_PAREN,
    UNKNOWN,
    END,
  };

  StenoKeyPressToken() = default;
  StenoKeyPressToken(Type type, uint32_t keyCode = 0)
      : type(type), keyCode(keyCode) {}

  bool operator==(const StenoKeyPressToken &other) const {
    if (type == Type::KEY) {
      if (keyCode != other.keyCode) {
        return false;
      }
    }
    return type == other.type;
  }

  Type type;
  uint32_t keyCode;
};

//---------------------------------------------------------------------------

// Helper class to help process commands such as {#a b Shift_L{a}}
class StenoKeyPressTokenizer {
public:
  StenoKeyPressTokenizer(const char *p, const char *end) : p(p), end(end) {
    ProcessNextToken();
  }

  bool HasMore() const {
    return nextToken.type != StenoKeyPressToken::Type::END;
  }

  StenoKeyPressToken GetNext() {
    StenoKeyPressToken result = nextToken;
    ProcessNextToken();
    return result;
  }

  StenoKeyPressToken::Type PeekNextTokenType() const { return nextToken.type; }

private:
  StenoKeyPressToken nextToken;

  const char *p;
  const char *end;

  void ProcessNextToken();
};

//---------------------------------------------------------------------------
