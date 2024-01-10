/**
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

#import "MCDataSource.h"

#include "core/Types.h"

#ifndef MCRotatingDeviceIdUniqueIdProvider_h
#define MCRotatingDeviceIdUniqueIdProvider_h

namespace matter {
namespace casting {
namespace support {

class MCRotatingDeviceIdUniqueIdProvider : public matter::casting::support::MutableByteSpanDataProvider
{
public:
    CHIP_ERROR Initialize(id<MCDataSource> dataSource);
    chip::MutableByteSpan * Get();

private:
    id<MCDataSource> mDataSource = nullptr;

    chip::MutableByteSpan mRotatingDeviceIdUniqueIdSpan;
    uint8_t mRotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MCRotatingDeviceIdUniqueIdProvider_h */
