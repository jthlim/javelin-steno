//---------------------------------------------------------------------------

#include "cyclic_queue.h"
#include "../unit_test.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

TEST_BEGIN("CyclicQueue: Iterator works as expected") {
  CyclicQueue<int, 4> intList;
  intList.Add(1);
  intList.Add(2);
  intList.Add(3);
  
  int sum = 0;
  for (int a : intList) {
    sum += a;
  }
  assert(sum == 6);
  
  intList.Add(4);
  sum = 0;
  for (int a : intList) {
    sum += a;
  }
  assert(sum == 10);
  
  intList.RemoveFront();
  sum = 0;
  for (int a : intList) {
    sum += a;
  }
  assert(sum == 9);
  
  intList.RemoveFront();
  sum = 0;
  for (int a : intList) {
    sum += a;
  }
  assert(sum == 7);
  
  intList.Add(5);
  sum = 0;
  for (int a : intList) {
    sum += a;
  }
  assert(sum == 12);
}
TEST_END

//---------------------------------------------------------------------------
