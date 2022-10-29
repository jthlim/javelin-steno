//---------------------------------------------------------------------------

#include "unit_test.h"
#include "key_code.h"
#include <stdio.h>

//---------------------------------------------------------------------------

#ifdef RUN_TESTS

std::vector<const UnitTest *> &UnitTest::GetTests() {
  static std::vector<const UnitTest *> tests;
  return tests;
}

UnitTest::UnitTest(void (*function)(), const char *name, const char *filename,
                   int lineNumber)
    : function(function), name(name), filename(filename),
      lineNumber(lineNumber) {
  GetTests().push_back(this);
}

void UnitTest::main() {
  for (const UnitTest *test : GetTests()) {
    try {
      Key::history.clear();
      (*test->function)();
      printf("[PASSED] %s\n", test->name);
    } catch (...) {
      printf("[FAILED] %s\n", test->name);
      printf("%s:%d: \n", test->filename, test->lineNumber);
    }
  }
}

__attribute__((weak)) int main(int argc, const char **argv) {
  UnitTest::main();
  return 0;
}

#endif

//---------------------------------------------------------------------------
