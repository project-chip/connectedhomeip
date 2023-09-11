/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <lega_rtos_api.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

class MatterInitializer
{
public:
    static CHIP_ERROR Init_Matter_Stack(const char * appName);
    static CHIP_ERROR Init_Matter_Server(void);
    static CHIP_ERROR Matter_Task_Config(lega_task_config_t * cfg);
};
