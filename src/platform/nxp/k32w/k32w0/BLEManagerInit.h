/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "ble_controller_task_config.h"
#include "ble_general.h"
#include "ble_host_task_config.h"
#include "controller_interface.h"

/* host task configuration */
#define HOST_TASK_PRIORITY (4U)
#define HOST_TASK_STACK_SIZE (gHost_TaskStackSize_c / sizeof(StackType_t))

typedef void (*ble_generic_cb_fp)(gapGenericEvent_t * pGenericEvent);

namespace chip {

class BLEManagerInit
{
public:
    static CHIP_ERROR InitHostController(ble_generic_cb_fp cb_fp);
    
private:
    static CHIP_ERROR blekw_host_init(void);
    static void Host_Task(osaTaskParam_t argument);
    static void BLE_SignalFromISRCallback(void);
    static CHIP_ERROR blekw_controller_init(void);
};

} // namespace chip

#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */
