/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_nrfconnect/init.h"

#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(app);

static LEDWidget sStatusLED;

namespace hdlc_example {
extern void Start();
} // namespace hdlc_example

int main()
{
    pw_sys_io_Init();

    LOG_INF("==================================================");
    LOG_INF("chip-nrf52840-pigweed-example starting");
#if BUILD_RELEASE
    LOG_INF("*** PSEUDO-RELEASE BUILD ***");
#else
    LOG_INF("*** DEVELOPMENT BUILD ***");
#endif
    LOG_INF("==================================================");

    // Light up the LED as a visual feedback that the flash was
    // successful.
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
    sStatusLED.Set(true);

    hdlc_example::Start();

    return 0;
}
