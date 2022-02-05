/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPVendorIdentifiers.hpp> // For VendorId
#include <lib/core/GroupId.h>
#include <lib/core/NodeId.h>
#include <lib/core/PasscodeId.h>

namespace chip {

typedef uint8_t ActionId;
typedef uint32_t AttributeId;
typedef uint32_t ClusterId;
typedef uint8_t ClusterStatus;
typedef uint32_t CommandId;
typedef uint32_t DataVersion;
typedef uint32_t DeviceTypeId;
typedef uint16_t EndpointId;
typedef uint32_t EventId;
typedef uint64_t EventNumber;
typedef uint64_t FabricId;
typedef uint8_t FabricIndex;
typedef uint32_t FieldId;
typedef uint16_t ListIndex;
typedef uint32_t TransactionId;
typedef uint16_t KeysetId;
typedef uint8_t InteractionModelRevision;

constexpr FabricIndex kUndefinedFabricIndex = 0;
constexpr EndpointId kInvalidEndpointId     = 0xFFFF;
constexpr EndpointId kRootEndpointId        = 0;
constexpr ListIndex kInvalidListIndex       = 0xFFFF; // List index is a uint16 thus 0xFFFF is a invalid list index.

// These are MEIs, 0xFFFF is not a valid manufacturer code,
// thus 0xFFFF'FFFF is not a valid MEI.
static constexpr ClusterId kInvalidClusterId     = 0xFFFF'FFFF;
static constexpr AttributeId kInvalidAttributeId = 0xFFFF'FFFF;
static constexpr CommandId kInvalidCommandId     = 0xFFFF'FFFF;
static constexpr EventId kInvalidEventId         = 0xFFFF'FFFF;
static constexpr FieldId kInvalidFieldId         = 0xFFFF'FFFF;

} // namespace chip
