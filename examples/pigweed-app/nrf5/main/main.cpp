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

#include <array>
#include <span>
#include <string_view>

#include "boards.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_baremetal_nrf528xx/init.h"

using std::byte;

namespace hdlc_example {
extern void Start();
} // namespace hdlc_example

namespace {
constexpr uint32_t kLedActive = LEDS_ACTIVE_STATE ? 1 : 0;
bool led_state                = true;

uint32_t LogTimestamp(void)
{
    return 0;
}

#define LOG_TIMESTAMP_FUNC LogTimestamp
#define LOG_TIMESTAMP_FREQ 1000

} // namespace

int main()
{
    ret_code_t ret;

    pw_sys_io_Init();

#if JLINK_MMD
    NVIC_SetPriority(DebugMonitor_IRQn, _PRIO_SD_LOWEST);
#endif

    // Initialize clock driver.
    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);

    // Wait for the clock to be ready.
    while (!nrf_clock_lf_is_running())
    {
    }

#if NRF_LOG_ENABLED

    // Initialize logging component
    ret = NRF_LOG_INIT(LOG_TIMESTAMP_FUNC, LOG_TIMESTAMP_FREQ);
    APP_ERROR_CHECK(ret);

    // Initialize logging backends
    NRF_LOG_DEFAULT_BACKENDS_INIT();

#endif

    NRF_LOG_INFO("==================================================");
    NRF_LOG_INFO("chip-nrf52840-pigweed-example starting");
#if BUILD_RELEASE
    NRF_LOG_INFO("*** PSEUDO-RELEASE BUILD ***");
#else
    NRF_LOG_INFO("*** DEVELOPMENT BUILD ***");
#endif
    NRF_LOG_INFO("==================================================");
    NRF_LOG_FLUSH();

    // Light up the LED as a visual feedback that the flash was
    // successful.
    nrf_gpio_cfg_output(SYSTEM_STATE_LED);
    nrf_gpio_pin_write(SYSTEM_STATE_LED, led_state ? kLedActive : !kLedActive);

    hdlc_example::Start();

    return 0;
}
