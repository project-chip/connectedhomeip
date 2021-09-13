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

#include <crypto/CHIPCryptoPAL.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_Zephyr.cpp>

#include <drivers/entropy.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

static K_THREAD_STACK_DEFINE(sChipThreadStack, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE);

PlatformManagerImpl PlatformManagerImpl::sInstance{ sChipThreadStack };

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    const struct device * entropy = device_get_binding(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);

    SuccessOrExit(entropy_get_entropy(entropy, output, len));

    *olen = len;

exit:
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::ZephyrConfig::Init();
    SuccessOrExit(err);

    // Add entropy source based on Zephyr entropy driver
    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
