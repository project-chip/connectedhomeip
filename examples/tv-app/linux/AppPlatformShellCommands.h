/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Contains shell commands for a ContentApp relating to Content App platform of the Video Player.
 */

#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
namespace chip {
namespace Shell {

void RegisterAppPlatformCommands();

} // namespace Shell
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
