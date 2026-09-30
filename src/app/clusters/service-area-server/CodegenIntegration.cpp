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

#include <app/clusters/service-area-server/CodegenIntegration.h>

#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <clusters/ServiceArea/Attributes.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

namespace {

ServiceAreaCluster::OptionalAttributeSet BuildOptionalAttributes(EndpointId endpointId)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    if (emberAfContainsAttribute(endpointId, Id, Attributes::SupportedMaps::Id))
    {
        optionalAttributes.template ForceSet<Attributes::SupportedMaps::Id>();
    }
    if (emberAfContainsAttribute(endpointId, Id, Attributes::CurrentArea::Id))
    {
        optionalAttributes.template ForceSet<Attributes::CurrentArea::Id>();
    }
    if (emberAfContainsAttribute(endpointId, Id, Attributes::EstimatedEndTime::Id))
    {
        optionalAttributes.template ForceSet<Attributes::EstimatedEndTime::Id>();
    }
    if (emberAfContainsAttribute(endpointId, Id, Attributes::Progress::Id))
    {
        optionalAttributes.template ForceSet<Attributes::Progress::Id>();
    }
    return optionalAttributes;
}

} // namespace

Instance::Instance(StorageDelegate * storageDelegate, Delegate * aDelegate, EndpointId aEndpointId,
                   BitMask<ServiceArea::Feature> aFeature) :
    mStorageDelegate(storageDelegate),
    mDelegate(aDelegate), mCluster(aEndpointId, *storageDelegate, *aDelegate, aFeature, BuildOptionalAttributes(aEndpointId))
{
    ChipLogProgress(Zcl, "Service Area: Instance constructor");
    aDelegate->SetInstance(this);
}

Instance::~Instance()
{
    if (mRegistered)
    {
        ChipLogError(AppServer, "Service Area Instance destroyed without Init() completing shutdown; unregistering now.");
        LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
        mRegistered = false;
    }
}

CHIP_ERROR Instance::Init()
{
    ChipLogProgress(Zcl, "Service Area: INIT");

    VerifyOrReturnError(emberAfContainsServer(mCluster.Cluster().GetPaths()[0].mEndpointId, Id), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "Service Area: The cluster with Id %lu was not enabled in zap.", long(Id)));

    VerifyOrReturnError(!mRegistered, CHIP_NO_ERROR);
    ReturnErrorOnFailure(mStorageDelegate->Init());
    ReturnErrorOnFailure(mDelegate->Init());
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration()));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

uint32_t Instance::GetNumberOfSupportedAreas()
{
    return mCluster.Cluster().GetNumberOfSupportedAreas();
}

bool Instance::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mCluster.Cluster().GetSupportedAreaByIndex(listIndex, aSupportedArea);
}

bool Instance::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mCluster.Cluster().GetSupportedAreaById(aAreaId, listIndex, aSupportedArea);
}

bool Instance::AddSupportedArea(AreaStructureWrapper & aNewArea)
{
    return mCluster.Cluster().AddSupportedArea(aNewArea);
}

bool Instance::ModifySupportedArea(AreaStructureWrapper & aNewArea)
{
    return mCluster.Cluster().ModifySupportedArea(aNewArea);
}

bool Instance::ClearSupportedAreas()
{
    return mCluster.Cluster().ClearSupportedAreas();
}

bool Instance::RemoveSupportedArea(uint32_t areaId)
{
    return mCluster.Cluster().RemoveSupportedArea(areaId);
}

uint32_t Instance::GetNumberOfSupportedMaps()
{
    return mCluster.Cluster().GetNumberOfSupportedMaps();
}

bool Instance::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    return mCluster.Cluster().GetSupportedMapByIndex(listIndex, aSupportedMap);
}

bool Instance::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    return mCluster.Cluster().GetSupportedMapById(aMapId, listIndex, aSupportedMap);
}

bool Instance::AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName)
{
    return mCluster.Cluster().AddSupportedMap(aMapId, aMapName);
}

bool Instance::RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName)
{
    return mCluster.Cluster().RenameSupportedMap(aMapId, newMapName);
}

bool Instance::ClearSupportedMaps()
{
    return mCluster.Cluster().ClearSupportedMaps();
}

bool Instance::RemoveSupportedMap(uint32_t mapId)
{
    return mCluster.Cluster().RemoveSupportedMap(mapId);
}

uint32_t Instance::GetNumberOfSelectedAreas()
{
    return mCluster.Cluster().GetNumberOfSelectedAreas();
}

bool Instance::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    return mCluster.Cluster().GetSelectedAreaByIndex(listIndex, selectedArea);
}

bool Instance::AddSelectedArea(uint32_t & aSelectedArea)
{
    return mCluster.Cluster().AddSelectedArea(aSelectedArea);
}

bool Instance::ClearSelectedAreas()
{
    return mCluster.Cluster().ClearSelectedAreas();
}

bool Instance::RemoveSelectedAreas(uint32_t areaId)
{
    return mCluster.Cluster().RemoveSelectedAreas(areaId);
}

DataModel::Nullable<uint32_t> Instance::GetCurrentArea()
{
    return mCluster.Cluster().GetCurrentArea();
}

bool Instance::SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea)
{
    return mCluster.Cluster().SetCurrentArea(aCurrentArea);
}

DataModel::Nullable<uint32_t> Instance::GetEstimatedEndTime()
{
    return mCluster.Cluster().GetEstimatedEndTime();
}

bool Instance::SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    return mCluster.Cluster().SetEstimatedEndTime(aEstimatedEndTime);
}

uint32_t Instance::GetNumberOfProgressElements()
{
    return mCluster.Cluster().GetNumberOfProgressElements();
}

bool Instance::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    return mCluster.Cluster().GetProgressElementByIndex(listIndex, aProgressElement);
}

bool Instance::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    return mCluster.Cluster().GetProgressElementById(aAreaId, listIndex, aProgressElement);
}

bool Instance::AddPendingProgressElement(uint32_t aAreaId)
{
    return mCluster.Cluster().AddPendingProgressElement(aAreaId);
}

bool Instance::SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus)
{
    return mCluster.Cluster().SetProgressStatus(aAreaId, opStatus);
}

bool Instance::SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    return mCluster.Cluster().SetProgressTotalOperationalTime(aAreaId, aTotalOperationalTime);
}

bool Instance::SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    return mCluster.Cluster().SetProgressEstimatedTime(aAreaId, aEstimatedTime);
}

bool Instance::ClearProgress()
{
    return mCluster.Cluster().ClearProgress();
}

bool Instance::RemoveProgressElement(uint32_t areaId)
{
    return mCluster.Cluster().RemoveProgressElement(areaId);
}

bool Instance::HasFeature(ServiceArea::Feature feature) const
{
    return mCluster.Cluster().HasFeature(feature);
}

// ZAP-generated plugin callbacks are left as stubs. Applications instantiate Instance directly
// (not through these callbacks) and register it with the codegen data model provider via Init().
void MatterServiceAreaClusterInitCallback(EndpointId endpointId) {}
void MatterServiceAreaClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType) {}
