/*
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
Error MapErrorPOSIX(int aError)
{
    return (aError == 0 ? CHIP_SYSTEM_NO_ERROR : aError);
}
} // namespace System
} // namespace chip

void SystemMutex_test()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    Mutex mLock;
    Error result = Mutex::Init(mLock);
    assert(result == CHIP_SYSTEM_NO_ERROR);
    mLock.Lock();
    mLock.Unlock();
}

void SystemClock_basic_test()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    uint64_t time = Platform::Layer::GetClock_Monotonic();
    assert(time);
}

int main()
{
    printf("---Running Test--- tests from %s\n", __FILE__);
    SystemMutex_test();
    SystemClock_basic_test();
    return 0;
}
