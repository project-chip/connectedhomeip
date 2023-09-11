/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct PlatformVersion
{
    uint8_t mMajor;
    uint8_t mMinor;
};

class SystemInfo
{
public:
    static CHIP_ERROR GetPlatformVersion(PlatformVersion & version);

private:
    static SystemInfo sInstance;
    uint8_t mMajor = 0;
    uint8_t mMinor = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
