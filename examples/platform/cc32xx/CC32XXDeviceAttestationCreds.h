/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace Credentials {
namespace CC32XX {

/**
 * @brief Get implementation of a sample DAC provider to validate device
 *        attestation procedure.
 *
 * @returns a singleton DeviceAttestationCredentialsProvider that relies on no
 *          storage abstractions.
 */
DeviceAttestationCredentialsProvider * GetCC32XXDacProvider();

} // namespace CC32XX
} // namespace Credentials
} // namespace chip
