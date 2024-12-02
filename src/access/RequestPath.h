/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>
#include <optional>

namespace chip {
namespace Access {

enum class RequestType : uint8_t
{
    kRequestTypeUnknown,
    kAttributeReadRequest,
    kAttributeWriteRequest,
    kCommandInvokeRequest,
    kEventReadRequest
};

struct RequestPath
{
    // NOTE: eventually this will likely also contain node, for proxying
    ClusterId cluster       = 0;
    EndpointId endpoint     = 0;
    RequestType requestType = RequestType::kRequestTypeUnknown;

    // entityId represents an attribute, command, or event ID, which is determined by the requestType. Wildcard if omitted.
    std::optional<uint32_t> entityId;
};

} // namespace Access
} // namespace chip
