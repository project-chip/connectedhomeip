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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for Zephyr platforms.
 */

#if !defined(CONFIG_NRF_SECURITY)
#include <crypto/CHIPCryptoPAL.h> // nogncheck
#endif                            // !defined(CONFIG_NRF_SECURITY)

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <platform/Zephyr/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_Zephyr.ipp>

#include <malloc.h>
#include <zephyr/drivers/entropy.h>

namespace chip {
namespace DeviceLayer {

static K_THREAD_STACK_DEFINE(sChipThreadStack, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE);

PlatformManagerImpl PlatformManagerImpl::sInstance{ sChipThreadStack };

static k_timer sOperationalHoursSavingTimer;

#if !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)
static bool sChipStackEntropySourceAdded = false;
static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    const struct device * entropy = DEVICE_DT_GET(DT_CHOSEN(zephyr_entropy));
    uint16_t clampedLen;
    if (CanCastTo<uint16_t>(len))
    {
        clampedLen = static_cast<uint16_t>(len);
    }
    else
    {
        clampedLen = UINT16_MAX;
    }
    int ret = entropy_get_entropy(entropy, output, clampedLen);

    if (ret == 0)
    {
        *olen = clampedLen;
    }
    else
    {
        *olen = 0;
    }

    return ret;
}
#endif // !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)

void PlatformManagerImpl::OperationalHoursSavingTimerEventHandler(k_timer * timer)
{
    PlatformMgr().ScheduleWork([](intptr_t arg) {
        CHIP_ERROR error = sInstance.UpdateOperationalHours(nullptr);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to update operational hours: %" CHIP_ERROR_FORMAT, error.Format());
        }
    });
}

CHIP_ERROR PlatformManagerImpl::UpdateOperationalHours(uint32_t * totalOperationalHours)
{
    uint64_t upTimeS;

    ReturnErrorOnFailure(GetDiagnosticDataProvider().GetUpTime(upTimeS));

    uint32_t totalTime       = 0;
    const uint32_t upTimeH   = upTimeS / 3600 < UINT32_MAX ? static_cast<uint32_t>(upTimeS / 3600) : UINT32_MAX;
    const uint64_t deltaTime = upTimeH - mSavedOperationalHoursSinceBoot;

    ReturnErrorOnFailure(ConfigurationMgr().GetTotalOperationalHours(totalTime));

    totalTime = totalTime + deltaTime < UINT32_MAX ? static_cast<uint32_t>(totalTime + deltaTime) : UINT32_MAX;

    if (deltaTime > 0)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalTime);
        mSavedOperationalHoursSinceBoot = upTimeH;
    }

    if (totalOperationalHours != nullptr)
    {
        *totalOperationalHours = totalTime;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

#if !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)
    // Minimum required from source before entropy is released ( with mbedtls_entropy_func() ) (in bytes)
    const size_t kThreshold = 16;
#endif // !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)

    // Initialize the configuration system.
    err = Internal::ZephyrConfig::Init();
    SuccessOrExit(err);

#if !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)
    if (!sChipStackEntropySourceAdded)
    {
        // Add entropy source based on Zephyr entropy driver
        err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, kThreshold);
        SuccessOrExit(err);
        sChipStackEntropySourceAdded = true;
    }
#endif // !defined(CONFIG_NRF_SECURITY) && !defined(CONFIG_MBEDTLS_ZEPHYR_ENTROPY) && !defined(CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR)

    // Call _InitChipStack() on the generic implementation base class to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    // Start the timer to periodically save node operational hours.
    k_timer_init(&sOperationalHoursSavingTimer, &PlatformManagerImpl::OperationalHoursSavingTimerEventHandler, nullptr);
    k_timer_user_data_set(&sOperationalHoursSavingTimer, this);
    k_timer_start(&sOperationalHoursSavingTimer, K_HOURS(CONFIG_CHIP_OPERATIONAL_TIME_SAVE_INTERVAL),
                  K_HOURS(CONFIG_CHIP_OPERATIONAL_TIME_SAVE_INTERVAL));

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
