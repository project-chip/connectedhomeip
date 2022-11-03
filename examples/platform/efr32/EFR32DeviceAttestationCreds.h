/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

// The "sl_token_manager.h" include belongs to the .cpp file, but the formatter change the order
// of the headers, causing a compilation error, so the include had to be added here instead
#include "sl_token_manager.h"
#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace Credentials {
namespace EFR32 {

/**
 * @brief Get implementation of a sample DAC provider to validate device
 *        attestation procedure.
 *
 * @returns a singleton DeviceAttestationCredentialsProvider that relies on no
 *          storage abstractions.
 */
DeviceAttestationCredentialsProvider * GetEFR32DacProvider();

} // namespace EFR32
} // namespace Credentials
} // namespace chip
