/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "MCRotatingDeviceIdUniqueIdProvider.h"

#include "lib/support/logging/CHIPLogging.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#import <Foundation/Foundation.h>

namespace matter {
namespace casting {
    namespace support {

        CHIP_ERROR MCRotatingDeviceIdUniqueIdProvider::Initialize(id<MCDataSource> dataSource)
        {
            VerifyOrReturnError(dataSource != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(mDataSource == nullptr, CHIP_ERROR_INCORRECT_STATE);
            mDataSource = dataSource;
            return CHIP_NO_ERROR;
        }

        chip::MutableByteSpan * MCRotatingDeviceIdUniqueIdProvider::Get()
        {
            ChipLogProgress(AppServer, "MCRotatingDeviceIdUniqueIdProvider.Get() called");
            VerifyOrReturnValue(mDataSource != nil, nullptr, ChipLogError(AppServer, "mDataSource found nil!"));

            NSData * uniqueIdData =
                [mDataSource castingAppDidReceiveRequestForRotatingDeviceIdUniqueId:@"MCRotatingDeviceIdUniqueIdProvider.Get()"];
            if (uniqueIdData != nil) {
                mRotatingDeviceIdUniqueIdSpan = chip::MutableByteSpan(
                    const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(uniqueIdData.bytes)), uniqueIdData.length);
            }
            return &mRotatingDeviceIdUniqueIdSpan;
        }

    }; // namespace support
}; // namespace casting
}; // namespace matter
