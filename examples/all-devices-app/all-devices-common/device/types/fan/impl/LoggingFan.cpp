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

#include <device/types/fan/impl/LoggingFan.h>
#include <devices/Types.h>

namespace chip {
namespace app {

LoggingFan::LoggingFan(const Context & context) :
    LoggingFanLoad(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kFan, 1), context)
{}

LoggingFan::LoggingFan(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context) :
    LoggingFanLoad(deviceTypes, context)
{}

} // namespace app
} // namespace chip
