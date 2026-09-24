/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "service-area-cluster-objects.h"
#include "service-area-delegate.h"
#include "service-area-storage-delegate.h"

#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

/**
 * Legacy wrapper around ServiceAreaCluster for backwards compatibility with existing applications.
 *
 * NEW CODE should use ServiceAreaCluster directly (see ServiceAreaCluster.h), which integrates cleanly
 * with the code-driven data model and does not carry the Ember/ZAP compatibility overhead.
 *
 * The Instance name is retained so that existing delegate and application code continues to compile
 * without modification.
 */
class Instance
{
public:
    /**
     * @brief Creates a Service Area cluster instance. The Init() method needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param[in] storageDelegate A pointer to the storage delegate to be used by this server.
     * @param[in] aDelegate A pointer to the delegate to be used by this server.
     * @param[in] aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param[in] aFeature The supported features of this Service Area Cluster.
     *
     * @note the caller must ensure that the delegates live throughout the instance's lifetime.
     */
    Instance(StorageDelegate * storageDelegate, Delegate * aDelegate, EndpointId aEndpointId,
             BitMask<ServiceArea::Feature> aFeature);

    ~Instance();

    Instance(const Instance &)             = delete;
    Instance & operator=(const Instance &) = delete;

    /**
     * @brief Initialise the Service Area server instance.
     * @return CHIP_NO_ERROR if there are on errors. Returns an error if
     *   - the given endpoint and cluster ID have not been enabled in zap
     *   - if registration with the data model provider fails
     *   - if the StorageDelegate or Delegate initialisation fails.
     */
    CHIP_ERROR Init();

    // Supported Areas accessors and manipulators
    uint32_t GetNumberOfSupportedAreas();
    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea);
    bool GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea);
    bool AddSupportedArea(AreaStructureWrapper & aNewArea);
    bool ModifySupportedArea(AreaStructureWrapper & aNewArea);
    bool ClearSupportedAreas();
    bool RemoveSupportedArea(uint32_t areaId);

    // Supported Maps accessors and manipulators
    uint32_t GetNumberOfSupportedMaps();
    bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap);
    bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap);
    bool AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName);
    bool RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName);
    bool ClearSupportedMaps();
    bool RemoveSupportedMap(uint32_t mapId);

    // Selected Areas accessors and manipulators
    uint32_t GetNumberOfSelectedAreas();
    bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea);
    bool AddSelectedArea(uint32_t & aSelectedArea);
    bool ClearSelectedAreas();
    bool RemoveSelectedAreas(uint32_t areaId);

    // Current Area accessors and manipulators
    DataModel::Nullable<uint32_t> GetCurrentArea();
    bool SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea);

    // Estimated End Time accessors and manipulators
    DataModel::Nullable<uint32_t> GetEstimatedEndTime();
    bool SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime);

    // Progress list accessors and manipulators
    uint32_t GetNumberOfProgressElements();
    bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement);
    bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement);
    bool AddPendingProgressElement(uint32_t aAreaId);
    bool SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus);
    bool SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime);
    bool SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime);
    bool ClearProgress();
    bool RemoveProgressElement(uint32_t areaId);

    // Feature Map attribute
    bool HasFeature(ServiceArea::Feature feature) const;

private:
    StorageDelegate * mStorageDelegate;
    Delegate * mDelegate;
    bool mRegistered = false;
    RegisteredServerCluster<ServiceAreaCluster> mCluster;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
