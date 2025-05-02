//---------------------------------------------------------------------------

#ifdef RUN_TESTS

//---------------------------------------------------------------------------

#include "unit_test.h"
#include "console.h"
#include "key.h"
#include <stdio.h>

//---------------------------------------------------------------------------

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
  size_t passedCount = 0;
  size_t failedCount = 0;
  for (const UnitTest *test : GetTests()) {
    try {
      Console::history.clear();
      Key::history.clear();
      (*test->function)();
      passedCount++;
      printf("[PASSED] %s\n", test->name);
    } catch (...) {
      printf("[FAILED] %s\n", test->name);
      printf("%s:%d: \n", test->filename, test->lineNumber);
      failedCount++;
    }
  }
  printf("\nFinished %zu tests, %zu passed, %zu failed\n",
         passedCount + failedCount, passedCount, failedCount);
}

[[gnu::weak]] int main(int argc, const char **argv) {
  UnitTest::main();
  return 0;
}

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------
