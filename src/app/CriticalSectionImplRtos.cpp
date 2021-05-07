/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "CriticalSection.h"
#include "FreeRTOS.h"
#include "task.h"
#include <pthread.h>
namespace chip {
namespace app {
static pthread_mutex_t eventLocker = PTHREAD_MUTEX_INITIALIZER;
void CriticalSectionEnter()
{
    int err = pthread_mutex_lock(&eventLocker);
    assert(err == 0);
    // TODO: Add freertos lock, it seems in EFR32, it needs portENTER_CRITICAL, in ESP32, it needs portENTER_CRITICAL(mux)
    // taskENTER_CRITICAL();
}

void CriticalSectionExit()
{
    int err = pthread_mutex_unlock(&eventLocker);
    assert(err == 0);
    // taskEXIT_CRITICAL();
}
} // namespace app
} // namespace chip
