/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemLayer.h>

namespace chip {
namespace System {

CHIP_ERROR Layer::ScheduleLambdaBridge(LambdaBridge && bridge)
{
    CHIP_ERROR lReturn = PlatformEventing::ScheduleLambdaBridge(*this, std::move(bridge));
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "Failed to queue CHIP System Layer lambda event: %" CHIP_ERROR_FORMAT, lReturn.Format());
    }
    return lReturn;
}

} // namespace System
} // namespace chip
