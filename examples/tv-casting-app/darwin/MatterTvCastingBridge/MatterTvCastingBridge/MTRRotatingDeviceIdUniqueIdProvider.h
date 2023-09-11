/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDataSource.h"

#include "core/Types.h"

#ifndef MTRRotatingDeviceIdUniqueIdProvider_h
#define MTRRotatingDeviceIdUniqueIdProvider_h

namespace matter {
namespace casting {
namespace support {

class MTRRotatingDeviceIdUniqueIdProvider : public matter::casting::support::MutableByteSpanDataProvider
{
public:
    CHIP_ERROR Initialize(id<MTRDataSource> dataSource);
    chip::MutableByteSpan * Get();

private:
    id<MTRDataSource> mDataSource = nullptr;

    chip::MutableByteSpan mRotatingDeviceIdUniqueIdSpan;
    uint8_t mRotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MTRRotatingDeviceIdUniqueIdProvider_h */
