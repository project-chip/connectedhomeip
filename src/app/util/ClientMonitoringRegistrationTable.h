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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

namespace chip {

/**
 * @brief ClientMonitoringRegistrationTable exists to manage the persistence of entries in the ClientMonitoring Cluster.
 *        To access persisted data with the ClientMonitoringRegistrationTable class, instantiate an instance of this class
 *        and call the LoadFromStorage function.
 *
 *        This class can only manage one fabric at a time. The flow is load a fabric, execute necessary operations,
 *        save it if there are any changes and load another fabric.
 */
class ClientMonitoringRegistrationTable
{
public:
    using MonitoringRegistrationStruct = chip::app::Clusters::ClientMonitoring::Structs::MonitoringRegistration::Type;

    struct ClientRegistrationEntry : MonitoringRegistrationStruct
    {
        bool IsValid() { return clientNodeId != kUndefinedNodeId && ICid != kInvalidIcId && fabricIndex != kUndefinedFabricIndex; }
    };

    ClientMonitoringRegistrationTable(PersistentStorageDelegate & storage);
    ~ClientMonitoringRegistrationTable(){};

    /**
     * @brief Function saves the mRegisteredClient attribute to persitant storage
     *        To correctly persit an entry, the values must be stored in the structures attributes
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR SaveToStorage();

    /**
     * @brief Function loads a client registration entry from persistent storage for a single fabric
     *
     * @param[in] fabricIndex fabric index to load from storage
     * @return CHIP_ERROR
     */
    CHIP_ERROR LoadFromStorage(FabricIndex fabricIndex);

    /**
     * @brief Accessor function that returns the client registration entry that was loaded for a fabric from persistant storage.
     * @see LoadFromStorage
     *
     * @return ClientMonitoringRegistrationTable::ClientRegistrationEntry&
     */
    ClientRegistrationEntry & GetClientRegistrationEntry();

private:
    static constexpr uint8_t kRegStorageSize = TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(uint64_t));

    ClientRegistrationEntry mRegisteredClient;
    PersistentStorageDelegate & mStorage;
};

} // namespace chip
