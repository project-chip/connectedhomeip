/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for the Texas Instruments CC1352 platform.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/PlatformManager.h>
#include <platform/cc13x2_26x2/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#include <lwip/tcpip.h>

#include <openthread/platform/entropy.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/* Include DMM module */
#include <dmm/dmm_policy.h>
#include <dmm/dmm_scheduler.h>

#include "ti_dmm_application_policy.h"
#include <dmm/dmm_priority_ble_thread.h>

TRNG_Handle TRNG_handle_app;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

static int app_get_random(uint8_t * aOutput, size_t aLen)
{
    int_fast16_t rtn;
    CryptoKey entropyKey;

    /*
     * prepare the data buffer
     */
    CryptoKeyPlaintext_initBlankKey(&entropyKey, aOutput, aLen);

    /* get entropy */
    rtn = TRNG_generateEntropy(TRNG_handle_app, &entropyKey);
    if (rtn != TRNG_STATUS_SUCCESS)
    {
        while (true)
            ;
    }
    return 0;
}

static void app_random_init(void)
{
    TRNG_Params TRNGParams;

    /* Init the TRNG HW */
    TRNG_init();

    TRNG_Params_init(&TRNGParams);
    /* use the polling mode */
    TRNGParams.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;

    TRNG_handle_app = TRNG_open(CONFIG_TRNG_APP, &TRNGParams);
    if (TRNG_handle_app == NULL)
    {
        while (true)
            ;
    }

    return;
}

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{

    app_get_random(reinterpret_cast<uint8_t *>(output), static_cast<uint16_t>(len));
    *olen = len;

    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::CC13X2_26X2Config::Init();
    SuccessOrExit(err);

    // DMM Addition
    DMMPolicy_Params dmmPolicyParams;
    DMMSch_Params dmmSchedulerParams;

    /* initialize and open the DMM policy manager */
    DMMPolicy_init();
    DMMPolicy_Params_init(&dmmPolicyParams);
    dmmPolicyParams.numPolicyTableEntries = DMMPolicy_ApplicationPolicySize;
    dmmPolicyParams.policyTable           = DMMPolicy_ApplicationPolicyTable;
    dmmPolicyParams.globalPriorityTable   = globalPriorityTable_bleLthreadH;
    DMMPolicy_open(&dmmPolicyParams);

    /* initialize and open the DMM scheduler */
    DMMSch_init();
    DMMSch_Params_init(&dmmSchedulerParams);

    // Copy stack roles and index table
    memcpy(dmmSchedulerParams.stackRoles, DMMPolicy_ApplicationPolicyTable.stackRole,
           sizeof(DMMPolicy_StackRole) * DMMPOLICY_NUM_STACKS);
    dmmSchedulerParams.indexTable = DMMPolicy_ApplicationPolicyTable.indexTable;
    DMMSch_open(&dmmSchedulerParams);

    // Initialize LwIP.
    tcpip_init(NULL, NULL);

    app_random_init();
    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
