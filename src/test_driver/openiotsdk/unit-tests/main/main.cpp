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

extern "C" {
#include "hal/serial_api.h"
#include "mps3_uart.h"
}

#include <platform/openiotsdk/Logging.h>
#include <stdio.h>
#include <stdlib.h>

#include <support/UnitTestRegistration.h>

using namespace ::chip;

mdh_serial_t * serial_ptr = NULL;

extern mdh_serial_t * get_serial()
{
    if (serial_ptr == NULL)
    {
        mps3_uart_t * uart;
        mps3_uart_init(&uart, &UART0_CMSDK_DEV_NS);
        serial_ptr = &(uart->serial);
        mdh_serial_set_baud(serial_ptr, 115200);
        return serial_ptr;
    }
    else
    {
        return serial_ptr;
    }
}

int main()
{
    chip::Logging::Platform::openiotsdk_logging_init();
    int status = RunRegisteredUnitTests();
    ChipLogProgress(NotSpecified, "Test status: %d", status);
    ChipLogProgress(NotSpecified, "Open IoT SDK unit-tests completed");

    return status;
}
