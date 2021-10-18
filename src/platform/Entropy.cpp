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
#include <crypto/RandUtils.h>
#include <lib/support/CodeUtils.h>

// Temporary includes for TemporaryAuditRandomPerformance()
// TODO: remove once https://github.com/project-chip/connectedhomeip/issues/10454 is done.
#include <lib/support/BytesToHex.h>

namespace chip {

namespace {

// Audit random number generator proper initialization with prints.
// TODO: remove once https://github.com/project-chip/connectedhomeip/issues/10454 is done.
void TemporaryAuditRandomNumberGenerator()
{
    uint8_t buf1[16] = { 0 };
    uint8_t buf2[16] = { 0 };

    VerifyOrDie(Crypto::DRBG_get_bytes(&buf1[0], sizeof(buf1)) == CHIP_NO_ERROR);
    VerifyOrDie(Crypto::DRBG_get_bytes(&buf2[0], sizeof(buf2)) == CHIP_NO_ERROR);

    char hex_buf[sizeof(buf1) * 2 + 1];

    ChipLogProgress(DeviceLayer, "AUDIT: ===== RANDOM NUMBER GENERATOR AUDIT START ====");
    ChipLogProgress(DeviceLayer, "AUDIT: * Validate buf1 and buf2 are <<<different every run/boot!>>>");
    ChipLogProgress(DeviceLayer, "AUDIT: * Validate r1 and r2 are <<<different every run/boot!>>>");

    memset(&hex_buf[0], 0, sizeof(hex_buf));
    VerifyOrDie(Encoding::BytesToUppercaseHexString(&buf1[0], sizeof(buf1), &hex_buf[0], sizeof(hex_buf)) == CHIP_NO_ERROR);
    ChipLogProgress(DeviceLayer, "AUDIT: * buf1: %s", &hex_buf[0]);

    memset(&hex_buf[0], 0, sizeof(hex_buf));
    VerifyOrDie(Encoding::BytesToUppercaseHexString(&buf2[0], sizeof(buf2), &hex_buf[0], sizeof(hex_buf)) == CHIP_NO_ERROR);
    ChipLogProgress(DeviceLayer, "AUDIT: * buf2: %s", &hex_buf[0]);

    VerifyOrDieWithMsg(memcmp(&buf1[0], &buf2[0], sizeof(buf1)) != 0, DeviceLayer,
                       "AUDIT: FAILED: buf1, buf2 are equal: DRBG_get_bytes() does not function!");

    uint32_t r1 = Crypto::GetRandU32();
    uint32_t r2 = Crypto::GetRandU32();

    ChipLogProgress(DeviceLayer, "AUDIT: * r1: 0x%08" PRIX32 " r2: 0x%08" PRIX32, r1, r2);
    VerifyOrDieWithMsg(r1 != r2, DeviceLayer, "AUDIT: FAILED: r1, r2 are equal: random number generator does not function!");
    ChipLogProgress(DeviceLayer, "AUDIT: ===== RANDOM NUMBER GENERATOR AUDIT END ====");
}

} // namespace

namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitEntropy()
{
    unsigned int seed;
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes((uint8_t *) &seed, sizeof(seed)));
    srand(seed);

    // TODO: remove once https://github.com/project-chip/connectedhomeip/issues/10454 is done.
    TemporaryAuditRandomNumberGenerator();
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
