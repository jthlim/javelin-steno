//---------------------------------------------------------------------------

#include "orthography.h"
#include "console.h"
#include "str.h"

//---------------------------------------------------------------------------

constexpr StenoOrthography StenoOrthography::emptyOrthography = {
    .ruleCount = 0,
    .rules = nullptr,
    .aliasCount = 0,
    .aliases = nullptr,
};

//---------------------------------------------------------------------------

void StenoOrthography::Print() const {
  char buffer[256];

  Console::Printf("{\n");
  Console::Printf("\t\"rules\": [");
  for (size_t i = 0; i < ruleCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"pattern\": \"");
    char *p = Str::WriteJson(buffer, rules[i].testPattern);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"replacement\": \"");
    p = Str::WriteJson(buffer, rules[i].replacement);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t],");
  Console::Printf("\n\t\"aliases\": [");
  for (size_t i = 0; i < aliasCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"suffix\": \"");
    char *p = Str::WriteJson(buffer, aliases[i].text);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"alias\": \"");
    p = Str::WriteJson(buffer, aliases[i].alias);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t]\n}\n\n");
}

//---------------------------------------------------------------------------
