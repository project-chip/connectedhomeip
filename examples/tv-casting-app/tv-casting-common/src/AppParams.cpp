/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppParams.h"

#if CHIP_ENABLE_ROTATING_DEVICE_ID
AppParams::AppParams(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueId)
{
    mRotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
}

void AppParams::SetRotatingDeviceIdUniqueId(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueId)
{
    mRotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
}

chip::Optional<chip::ByteSpan> AppParams::GetRotatingDeviceIdUniqueId()
{
    return mRotatingDeviceIdUniqueId;
}
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
