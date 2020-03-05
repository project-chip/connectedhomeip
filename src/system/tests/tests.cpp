// This is a sample unit test.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "SystemMutex.cpp"

using namespace chip::System;

void SystemMutex_test() {
  printf("---Running Test--- %s\n", __FUNCTION__);
  Mutex mLock;
  Error result = Mutex::Init(mLock);
  assert(result == CHIP_SYSTEM_NO_ERROR);
  mLock.Lock();
  mLock.Unlock();
}

int main() {
  printf("---Running Test--- tests from %s\n", __FILE__);
  SystemMutex_test();
  return 0;
}