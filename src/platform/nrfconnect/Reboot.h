/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace chip {
namespace DeviceLayer {

enum class SoftwareRebootReason : uint8_t
{
    kSoftwareUpdate,
    kOther
};

[[noreturn]] void Reboot(SoftwareRebootReason reason);
SoftwareRebootReason GetSoftwareRebootReason();

} // namespace DeviceLayer
} // namespace chip
