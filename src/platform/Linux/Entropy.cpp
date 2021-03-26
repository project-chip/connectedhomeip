/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *          on the Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/crypto/CHIPRNG.h>

using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;
    unsigned int seed;

    // Initialize the source used by CHIP to get secure random data.
    err = Platform::Security::InitSecureRandomDataSource(getentropy, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    err = Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
    SuccessOrExit(err);
    srand(seed);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "InitEntropy() failed: %d" err);
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
