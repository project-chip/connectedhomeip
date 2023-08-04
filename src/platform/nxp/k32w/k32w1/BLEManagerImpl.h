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

#include "RNG_Interface.h"
#include "fwk_messaging.h"
#include "fwk_os_abs.h"
#include "fwk_platform.h"
#include "fwk_platform_ble.h"
#include "hci_transport.h"

#include "ble_init.h"
#include "controller_api.h"
#include "controller_interface.h"

#include <src/platform/nxp/k32w/common/BLEManagerCommon.h>

/* host task configuration */
#define HOST_TASK_PRIORITY (4U)
#define HOST_TASK_STACK_SIZE (gHost_TaskStackSize_c / sizeof(StackType_t))

#ifndef HCI_RESET_WAIT_TIME_US
#define HCI_RESET_WAIT_TIME_US 30000
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

class BLEManagerImpl final : public BLEManagerCommon
{
public:
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

    CHIP_ERROR InitHostController(BLECallbackDelegate::GapGenericCallback cb_fp) override;
    BLEManagerCommon * GetImplInstance() override;
    CHIP_ERROR ResetController() override;

private:
    static BLEManagerImpl sInstance;

    static CHIP_ERROR blekw_host_init(void);
    static void Host_Task(osaTaskParam_t argument);

    BleLayer * _GetBleLayer(void);

    // ===== Members for internal use by the following friends.
    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr(void)
{
    return BLEManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the K32W platforms.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */
