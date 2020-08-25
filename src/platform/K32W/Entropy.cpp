/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          Provides implementations for the Chip entropy sourcing functions
 *          on the NXP K32W platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/crypto/CHIPRNG.h>

#include <mbedtls/entropy_poll.h>
#include <openthread/platform/entropy.h>

using namespace ::chip;

#if !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
#error "CHIP DRBG implementation must be enabled on K32W platforms"
#endif // !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Retrieve entropy from the underlying RNG source.
 *
 * This function is called by the CHIP DRBG to acquire entropy.
 */
int GetEntropy_K32W(uint8_t * buf, size_t count)
{
    int res = 0;

    VerifyOrDie(count <= UINT16_MAX);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().LockThreadStack();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    otError otErr = otPlatEntropyGet(buf, (uint16_t) count);
    if (otErr != OT_ERROR_NONE)
    {
        res = CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().UnlockThreadStack();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    return res;
}

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;

    // Initialize the CHIP DRBG.
    err = Platform::Security::InitSecureRandomDataSource(GetEntropy_K32W, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    {
        unsigned int seed;
        err = Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
        SuccessOrExit(err);
        srand(seed);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "InitEntropy() failed: 0x%08" PRIX32, err);
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
