//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

// Used to parse a partitioned stream of data to detect if the command
// "start_javelin_console\n" is in the stream.
class StartConsoleCommandDetector {
public:
  // Returns the offset of the final '\n' in the command stream, -1 if not
  // found.
  size_t IsStartCommandPresent(const uint8_t *data, size_t length);

  void Reset() { state = 0; }

private:
  int state = 0;

  bool ProcessByte(uint8_t c);
};

//---------------------------------------------------------------------------
