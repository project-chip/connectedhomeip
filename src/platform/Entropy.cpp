/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/RandUtils.h>
#include <lib/support/CodeUtils.h>

namespace chip {

namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitEntropy()
{
    unsigned int seed;
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes((uint8_t *) &seed, sizeof(seed)));
    srand(seed);

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
