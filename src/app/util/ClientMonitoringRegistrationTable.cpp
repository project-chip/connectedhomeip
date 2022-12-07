/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ClientMonitoringRegistrationTable.h"

namespace chip {

/**********************************************************
 * Attributes Definition
 *********************************************************/

/**********************************************************
 * ClientMonitoringRegistrationTable Implementation
 *********************************************************/

ClientMonitoringRegistrationTable::ClientMonitoringRegistrationTable(FabricIndex fabricIndex)
{
    this->LoadFromStorage(fabricIndex);
}

void ClientMonitoringRegistrationTable::LoadFromStorage(FabricIndex fabricIndex)
{
    // TODO: Implement load from NVM logic
}

void ClientMonitoringRegistrationTable::SaveToStorage()
{
    // Store to NVM based of class attributes
}

NodeId ClientMonitoringRegistrationTable::getClientNodeId()
{
    return mRegisteredClient.clientNodeId;
}

uint64_t ClientMonitoringRegistrationTable::getICid()
{
    return mRegisteredClient.ICid;
}

FabricIndex ClientMonitoringRegistrationTable::getFaricIndex()
{
    return mRegisteredClient.fabricIndex;
}

void ClientMonitoringRegistrationTable::setClientNodeId(NodeId clientNodeId)
{
    mRegisteredClient.clientNodeId = clientNodeId;
}

void ClientMonitoringRegistrationTable::setICid(uint64_t ICid)
{
    mRegisteredClient.ICid = ICid;
}

void ClientMonitoringRegistrationTable::setFabricIndex(FabricIndex fabric)
{
    mRegisteredClient.fabricIndex = fabric;
}

} // namespace chip
