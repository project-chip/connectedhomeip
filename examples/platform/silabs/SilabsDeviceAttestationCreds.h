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
namespace Silabs {

/**
 * @brief Get implementation of a sample DAC provider to validate device
 *        attestation procedure.
 *
 * @returns a singleton DeviceAttestationCredentialsProvider that relies on no
 *          storage abstractions.
 */
DeviceAttestationCredentialsProvider * GetSilabsDacProvider();

void SilabsDacProviderMigration(void);

} // namespace Silabs
} // namespace Credentials
} // namespace chip
