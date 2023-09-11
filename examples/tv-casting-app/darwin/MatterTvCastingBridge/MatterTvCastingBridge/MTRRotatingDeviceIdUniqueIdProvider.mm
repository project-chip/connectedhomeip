/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MTRRotatingDeviceIdUniqueIdProvider.h"

#include "lib/support/logging/CHIPLogging.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#import <Foundation/Foundation.h>

namespace matter {
namespace casting {
    namespace support {

        CHIP_ERROR MTRRotatingDeviceIdUniqueIdProvider::Initialize(id<MTRDataSource> dataSource)
        {
            VerifyOrReturnError(dataSource != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(mDataSource == nullptr, CHIP_ERROR_INCORRECT_STATE);
            mDataSource = dataSource;
            return CHIP_NO_ERROR;
        }

        chip::MutableByteSpan * MTRRotatingDeviceIdUniqueIdProvider::Get()
        {
            ChipLogProgress(AppServer, "MTRRotatingDeviceIdUniqueIdProvider.Get() called");
            VerifyOrReturnValue(mDataSource != nil, nullptr, ChipLogError(AppServer, "mDataSource found nil!"));

            NSData * uniqueIdData =
                [mDataSource castingAppDidReceiveRequestForRotatingDeviceIdUniqueId:@"MTRRotatingDeviceIdUniqueIdProvider.Get()"];
            if (uniqueIdData != nil) {
                mRotatingDeviceIdUniqueIdSpan = chip::MutableByteSpan(
                    const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(uniqueIdData.bytes)), uniqueIdData.length);
            }
            return &mRotatingDeviceIdUniqueIdSpan;
        }

    }; // namespace support
}; // namespace casting
}; // namespace matter
