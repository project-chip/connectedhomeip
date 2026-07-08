/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <devices/Types.h>
#include <devices/fan/impl/LoggingFanDevice.h>

namespace chip {
namespace app {

LoggingFanDevice::LoggingFanDevice(const Context & context) :
    LoggingFanLoadDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kFan, 1), context)
{}

LoggingFanDevice::LoggingFanDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context) :
    LoggingFanLoadDevice(deviceTypes, context)
{}

} // namespace app
} // namespace chip
