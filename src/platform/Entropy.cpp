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

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitEntropy()
{
    unsigned int seed;
    ReturnErrorOnFailure(chip::Crypto::DRBG_get_bytes((uint8_t *) &seed, sizeof(seed)));
    srand(seed);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
