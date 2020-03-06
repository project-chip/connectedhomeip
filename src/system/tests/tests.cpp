// This is a sample unit test.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "SystemClock.cpp"
#include "SystemMutex.cpp"

using namespace chip::System;

namespace chip {
namespace System {
Error MapErrorPOSIX(int aError) {
  return (aError == 0 ? CHIP_SYSTEM_NO_ERROR : aError);
}
}  // namespace System
}  // namespace chip

void SystemMutex_test() {
  printf("---Running Test--- %s\n", __FUNCTION__);
  Mutex mLock;
  Error result = Mutex::Init(mLock);
  assert(result == CHIP_SYSTEM_NO_ERROR);
  mLock.Lock();
  mLock.Unlock();
}

void SystemClock_basic_test() {
  printf("---Running Test--- %s\n", __FUNCTION__);
  uint64_t time = Platform::Layer::GetClock_Monotonic();
  assert(time);
}

int main() {
  printf("---Running Test--- tests from %s\n", __FILE__);
  SystemMutex_test();
  SystemClock_basic_test();
  return 0;
}