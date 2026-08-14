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

#include "ThermostatSensorStructWithOwnedMembers.h"
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatSensorStructWithOwnedMembers::ThermostatSensorStructWithOwnedMembers(const Structs::ThermostatSensorStruct::Type & other)
{
    *this = other;
}

ThermostatSensorStructWithOwnedMembers &
ThermostatSensorStructWithOwnedMembers::operator=(const Structs::ThermostatSensorStruct::Type & other)
{
    SetCluster(other.cluster);
    SetEndpoint(other.endpoint);
    SetNode(other.node);
    SetFabricIndex(other.fabricIndex);
    TEMPORARY_RETURN_IGNORED SetName(other.name);
    TEMPORARY_RETURN_IGNORED SetSensorHandle(other.sensorHandle);
    return *this;
}

ThermostatSensorStructWithOwnedMembers &
ThermostatSensorStructWithOwnedMembers::operator=(const ThermostatSensorStructWithOwnedMembers & other)
{
    if (this == &other)
    {
        return *this;
    }
    *this = static_cast<const ThermostatSensorStructWithOwnedMembers::Type &>(other);
    return *this;
}

CHIP_ERROR ThermostatSensorStructWithOwnedMembers::SetName(const CharSpan & newName)
{
    if (newName.size() > sizeof(mNameData))
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    memcpy(mNameData, newName.data(), newName.size());
    name = CharSpan(mNameData, newName.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatSensorStructWithOwnedMembers::SetSensorHandle(const ByteSpan & newSensorHandle)
{
    if (newSensorHandle.size() > sizeof(mSensorHandleData))
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    memcpy(mSensorHandleData, newSensorHandle.data(), newSensorHandle.size());
    sensorHandle = ByteSpan(mSensorHandleData, newSensorHandle.size());
    return CHIP_NO_ERROR;
}

void ThermostatSensorStructWithOwnedMembers::SetCluster(ClusterId newCluster)
{
    cluster = newCluster;
}

void ThermostatSensorStructWithOwnedMembers::SetEndpoint(const Optional<EndpointId> & newEndpoint)
{
    endpoint = newEndpoint;
}

void ThermostatSensorStructWithOwnedMembers::SetNode(const Optional<NodeId> & newNode)
{
    node = newNode;
}

void ThermostatSensorStructWithOwnedMembers::SetFabricIndex(const Optional<FabricIndex> & newFabricIndex)
{
    fabricIndex = newFabricIndex;
}

CharSpan ThermostatSensorStructWithOwnedMembers::GetName() const
{
    return name;
}

ByteSpan ThermostatSensorStructWithOwnedMembers::GetSensorHandle() const
{
    return sensorHandle;
}

ClusterId ThermostatSensorStructWithOwnedMembers::GetCluster() const
{
    return cluster;
}

Optional<EndpointId> ThermostatSensorStructWithOwnedMembers::GetEndpoint() const
{
    return endpoint;
}

Optional<NodeId> ThermostatSensorStructWithOwnedMembers::GetNode() const
{
    return node;
}

Optional<FabricIndex> ThermostatSensorStructWithOwnedMembers::GetFabricIndex() const
{
    return fabricIndex;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
