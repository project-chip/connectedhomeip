/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides implementations for the CHIP entropy sourcing functions
 *          on the PSoC6 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "cyhal_trng.h"

using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

cyhal_trng_t trng;

int GetEntropy_P6(uint8_t * buf, size_t bufSize)
{
    while (bufSize > 0)
    {
        uint32_t val = cyhal_trng_generate(&trng);
        size_t n     = chip::min(bufSize, sizeof(uint32_t));
        memcpy(buf, static_cast<void *>(&val), n);

        buf += n;
        bufSize -= n;
    }

    return 0;
}

} // unnamed namespace

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;

    err = (cyhal_trng_init(&trng) == CY_RSLT_SUCCESS) ? CHIP_NO_ERROR : CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED;
    SuccessOrExit(err);

    // Initialize the source used by Chip to get secure random data.
    err = ::chip::Platform::Security::InitSecureRandomDataSource(GetEntropy_P6, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    unsigned int seed;
    err = ::chip::Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
    SuccessOrExit(err);
    srand(seed);

exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
