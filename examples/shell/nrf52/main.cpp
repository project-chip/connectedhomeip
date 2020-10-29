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

#include <platform/CHIPDeviceConfig.h>

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#endif //#ifdef SOFTDEVICE_PRESENT

#if CHIP_ENABLE_OPENTHREAD
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}
#endif // CHIP_ENABLE_OPENTHREAD

using namespace chip;
using namespace chip::Shell;

// TODO: Move softdevice initialization behind platform interface.
static void OnSoCEvent(uint32_t sys_evt, void * p_context)
{
#if CHIP_ENABLE_OPENTHREAD
    otSysSoftdeviceSocEvtHandler(sys_evt);
#endif
    UNUSED_PARAMETER(p_context);
}

CHIP_ERROR soft_device_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = nrf_sdh_enable_request();
    SuccessOrExit(err);

    while (!nrf_sdh_is_enabled())
    {
    }

    // Register a handler for SOC events.
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, OnSoCEvent, NULL);

    {
        uint32_t appRAMStart = 0;

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        err = nrf_sdh_ble_default_cfg_set(CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG, &appRAMStart);
        SuccessOrExit(err);

        // Enable BLE stack.
        err = nrf_sdh_ble_enable(&appRAMStart);
        SuccessOrExit(err);
    }

exit:
    return err;
}

int main()
{
    soft_device_init();

    // Initialize the default streamer that was linked.
    const int rc = streamer_init(streamer_get());

    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return rc;
    }

    cmd_misc_init();
    cmd_base64_init();
    cmd_device_init();
    cmd_btp_init();
    cmd_otcli_init();

    shell_task(nullptr);
    return 0;
}
