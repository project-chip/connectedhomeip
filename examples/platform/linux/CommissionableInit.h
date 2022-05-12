/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include "LinuxCommissionableDataProvider.h"
#include "Options.h"
#include <lib/core/CHIPError.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace examples {

/**
 * @brief Initialize a LinuxCommissionableDataProvider from Linux common command-line
 *        options. Handles verifier, passcode, discriminator, etc.
 *
 * @param provider - provider to initialize from command line arguments
 * @param options - LinuxDeviceOptions instance configured via command-line parsing
 * @return CHIP_NO_ERROR on success or another CHIP_ERROR value on internal validation errors (likely fatal)
 */
CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options);

/**
 * @brief Initialize a Linux ConfigurationManagerImpl to reflect some command-line configured
 *        values such as VendorID/ProductID
 *
 * @param configManager - Linux-specific configuration manager to update
 * @param options - LinuxDeviceOptions instance configured via command-line parsing
 * @return CHIP_NO_ERROR on success or another CHIP_ERROR value on internal validation errors (likely fatal)
 */
CHIP_ERROR InitConfigurationManager(chip::DeviceLayer::ConfigurationManagerImpl & configManager, LinuxDeviceOptions & options);

} // namespace examples
} // namespace chip
