/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
