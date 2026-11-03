/**
 *
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

#pragma once

#include <stddef.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <clusters/Thermostat/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kThermostatSensorNameMaxSize   = 64;
static constexpr size_t kThermostatSensorHandleMaxSize = 16;

struct ThermostatSensorStructWithOwnedMembers : protected Structs::ThermostatSensorStruct::Type
{
public:
    ThermostatSensorStructWithOwnedMembers() = default;
    ThermostatSensorStructWithOwnedMembers(const Structs::ThermostatSensorStruct::Type & other);
    ThermostatSensorStructWithOwnedMembers(const ThermostatSensorStructWithOwnedMembers & other) = delete;
    ThermostatSensorStructWithOwnedMembers & operator=(const Structs::ThermostatSensorStruct::Type & other);
    ThermostatSensorStructWithOwnedMembers & operator=(const ThermostatSensorStructWithOwnedMembers & other);

    CHIP_ERROR SetName(const CharSpan & newName);
    CHIP_ERROR SetSensorHandle(const ByteSpan & newSensorHandle);
    void SetCluster(ClusterId newCluster);
    void SetEndpoint(const Optional<EndpointId> & newEndpoint);
    void SetNode(const Optional<NodeId> & newNode);
    void SetFabricIndex(const Optional<FabricIndex> & newFabricIndex);

    CharSpan GetName() const;
    ByteSpan GetSensorHandle() const;
    ClusterId GetCluster() const;
    Optional<EndpointId> GetEndpoint() const;
    Optional<NodeId> GetNode() const;
    Optional<FabricIndex> GetFabricIndex() const;

    using Structs::ThermostatSensorStruct::Type::Encode;
    using Structs::ThermostatSensorStruct::Type::kIsFabricScoped;

private:
    char mNameData[kThermostatSensorNameMaxSize]              = { 0 };
    uint8_t mSensorHandleData[kThermostatSensorHandleMaxSize] = { 0 };
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
