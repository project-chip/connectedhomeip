/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>

#if SILABS_LOG_OUT_UART
#include "uart.h"
#else
#include "SEGGER_RTT.h"
#endif

// TODO: The FreeRTOS CMSIS OS2 wrapper does not implement osKernelSuspend().
// Using vTaskSuspendAll() directly until the SDK provides the wrapper.
#include "FreeRTOS.h"
#include "task.h"

using chip::Encoding::HexFlags;
using chip::Encoding::Uint32ToHex;

extern "C" void chipDie(void)
{
    void * caller = __builtin_return_address(0);
    char msg[]    = "chipDie from 0x________\r\n";
    RETURN_SAFELY_IGNORED(Uint32ToHex(reinterpret_cast<uintptr_t>(caller), &msg[15], 8, HexFlags::kUppercase));

#if SILABS_LOG_OUT_UART
    uartForceTransmit(reinterpret_cast<const uint8_t *>(msg), sizeof(msg) - 1);
#else
    SEGGER_RTT_WriteNoLock(0, msg, sizeof(msg) - 1);
#endif

    vTaskSuspendAll();
    chipAbort();
}
