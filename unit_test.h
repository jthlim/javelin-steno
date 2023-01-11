//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <string>
#include <vector>

//---------------------------------------------------------------------------

class UnitTest {
public:
  UnitTest(void (*function)(), const char *name, const char *filename,
           int lineNumber);

  static void main();

private:
  void (*function)();
  const char *name;
  const char *filename;
  int lineNumber;

  static std::vector<const UnitTest *>& GetTests();
};

struct TestFailure {
  TestFailure(const std::string &message) : message(message) {}

  std::string message;
};

//---------------------------------------------------------------------------

#ifdef RUN_TESTS

// Put each unit test into its own namespace so the names can be referenced.
// Make all members static so there are no link-time conflicts between different
// files
#define TEST_BEGIN__(text, file, line)                                         \
  namespace UnitTest##line {                                                   \
    static const char *uTDescription = text;                                   \
    static const char *uTFile = file;                                          \
    static int uTLine = line;                                                  \
    static void Test() {

#define TEST_END                                                               \
  }                                                                            \
  static UnitTest test(&Test, uTDescription, uTFile, uTLine);                  \
  }

#else

#define TEST_BEGIN__(text, file, line)                                         \
  namespace UnitTest##line {                                                   \
    static void Test() {

#define TEST_END                                                               \
  }                                                                            \
  }

#endif

//---------------------------------------------------------------------------

#define TEST_BEGIN_(description, file, line)                                   \
  TEST_BEGIN__(description, file, line)

#define TEST_BEGIN(description) TEST_BEGIN_(description, __FILE__, __LINE__)

//---------------------------------------------------------------------------
