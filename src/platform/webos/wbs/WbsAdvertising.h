/* @@@LICENSE
 *
 * Copyright (c) 2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#pragma once

#include <cstdint>

#include <lib/core/CHIPError.h>
#include <luna-service2/lunaservice.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Drives the CHIPoBLE advertisement lifecycle over webOS's bluetooth2 LS2 service
/// (le/configureAdvertisement, le/startAdvertising, le/stopAdvertising).
class WbsAdvertising
{
public:
    WbsAdvertising()  = default;
    ~WbsAdvertising() = default;

    /// Configure and start CHIPoBLE advertising.
    CHIP_ERROR Start();

    /// Stop CHIPoBLE advertising. Completion is reported asynchronously via
    CHIP_ERROR Stop();

    bool IsAdvertising() const { return mIsAdvertising; }

private:
    static CHIP_ERROR StartImpl(WbsAdvertising * self);
    static CHIP_ERROR StopImpl(WbsAdvertising * self);
    static bool OnAdvertisingStatusChanged(LSHandle * sh, LSMessage * message, void * ctx);

    bool mIsAdvertising            = false;
    int32_t mAdvertiserId          = -1;
    LSMessageToken mSubscribeToken = LSMESSAGE_TOKEN_INVALID;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
