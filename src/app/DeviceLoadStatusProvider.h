/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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
#pragma once

#include <cstdint>

#include <app/SubscriptionStats.h>
#include <lib/core/DataModelTypes.h>
#include <transport/MessageStats.h>
#include <utility>

namespace chip {
namespace app {

class DeviceLoadStatusProvider
{
public:
    virtual ~DeviceLoadStatusProvider() = default;

    virtual MessageStats GetMessageStats() = 0;

    virtual SubscriptionStats GetSubscriptionStats(FabricIndex fabric) = 0;
};

} // namespace app
} // namespace chip
