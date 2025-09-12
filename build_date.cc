//---------------------------------------------------------------------------

#include "build_date.h"

//---------------------------------------------------------------------------

consteval static int GetMonth(char a, char b, char c) {
  if (a == 'J' && b == 'a' && c == 'n') {
    return 1;
  }
  if (a == 'F' && b == 'e' && c == 'b') {
    return 2;
  }
  if (a == 'M' && b == 'a' && c == 'r') {
    return 3;
  }
  if (a == 'A' && b == 'p' && c == 'r') {
    return 4;
  }
  if (a == 'M' && b == 'a' && c == 'y') {
    return 5;
  }
  if (a == 'J' && b == 'u' && c == 'n') {
    return 6;
  }
  if (a == 'J' && b == 'u' && c == 'l') {
    return 7;
  }
  if (a == 'A' && b == 'u' && c == 'g') {
    return 8;
  }
  if (a == 'S' && b == 'e' && c == 'p') {
    return 9;
  }
  if (a == 'O' && b == 'c' && c == 't') {
    return 10;
  }
  if (a == 'N' && b == 'o' && c == 'v') {
    return 11;
  }
  if (a == 'D' && b == 'e' && c == 'c') {
    return 12;
  }
  return 0;
}

//---------------------------------------------------------------------------

const char *GetBuildDate() {
  static constexpr char BUILD_DATE[] = {
      __DATE__[7],                                                //
      __DATE__[8],                                                //
      __DATE__[9],                                                //
      __DATE__[10],                                               //
      '-',                                                        //
      GetMonth(__DATE__[0], __DATE__[1], __DATE__[2]) / 10 + '0', //
      GetMonth(__DATE__[0], __DATE__[1], __DATE__[2]) % 10 + '0', //
      '-',                                                        //
      __DATE__[4] == ' ' ? '0' : __DATE__[4],                     //
      __DATE__[5],                                                //
      0,
  };

  return BUILD_DATE;
}

//---------------------------------------------------------------------------