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
 *          Provides implementations for the chip entropy sourcing functions
 *          on Qorvo QPG platforms.
 */

//
// !!!TEMPORARY CODE!!!
//
// The following code is a temporary implementation of the CHIP Entropy APIs
// that has been specially designed to work around the lack of thread-safety in the
// Nordic port of OpenThread.
//
// In Nordic's OpenThread platform implementation, the code assumes it has exclusive
// access to the underlying entropy source.  In a multi-threaded environment such as
// an CHIP application this precludes code running in other threads from directly
// sourcing entropy.
//
// To work around this, the code here acquires the OpenThread stack lock before interacting
// with the entropy source, effectively blocking all OpenThread activity until CHIP
// has acquired its entropy. Because the entropy is being fed into a DRBG, which then feeds
// the application, this should happen extremely rarely.
//
// Ultimately this code will be revised to use a new thread-safe entropy sourcing API
// provided by Nordic.
//

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/crypto/CHIPRNG.h>

#include <openthread/platform/random.h>

using namespace ::chip;

#if !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
#error "CHIP DRBG implementation must be enabled on Qorvo QPG platforms"
#endif // !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Retrieve entropy from the underlying RNG source.
 *
 * This function is called by the CHIP DRBG to acquire entropy.
 */
int GetEntropy(uint8_t * buf, size_t count)
{
    int res;

    VerifyOrDie(count <= UINT16_MAX);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // If OpenThread is active, acquire the stack lock to prevent the
    // OpenThread task from interacting with the entropy source.

    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().LockThreadStack();
    }

    // FIXME - use available HW based API
    // Call the OpenThread platform API for retrieving entropy.
    otError otErr = otPlatRandomGetTrue(buf, (uint16_t) count);
    res           = (otErr == OT_ERROR_NONE);

    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().UnlockThreadStack();
    }
#else
    // FIXME - use available HW based API - No OT needed in essence
    res = -1;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    return res;
}

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;

    // Initialize the CHIP DRBG.
    err = Platform::Security::InitSecureRandomDataSource(GetEntropy, 64, NULL, 0);
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
