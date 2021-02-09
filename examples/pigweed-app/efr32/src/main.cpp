/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppConfig.h"
#include "LEDWidget.h"
#include "init_board.h"
#include "init_mcu.h"

#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_efr32/init.h"
#include "uart.h"

static LEDWidget sStatusLED;

namespace hdlc_example {
extern void Start();
} // namespace hdlc_example

using namespace ::pw::sys_io;
int main(void)
{
    initMcu();
    initBoard();
    efr32LogInit();
    EFR32_LOG("***** CHIP EFR32 pigweed example *****\r\n");

    pw_sys_io_Init();
    // Initialize LEDs
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
    sStatusLED.Set(true);

    hdlc_example::Start();
}
