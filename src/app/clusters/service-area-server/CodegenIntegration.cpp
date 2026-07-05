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
    mStorageDelegate(storageDelegate), mDelegate(aDelegate),
    mCluster(aEndpointId, *storageDelegate, *aDelegate, aFeature, BuildOptionalAttributes(aEndpointId))
{
    ChipLogProgress(Zcl, "Service Area: Instance constructor");
    aDelegate->SetInstance(this);
}

Instance::~Instance()
{
    if (mRegistered)
    {
        ChipLogError(AppServer, "Service Area Instance destroyed without Init() completing shutdown; unregistering now.");
        TEMPORARY_RETURN_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
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

#define FORWARD_TO_CLUSTER(method) return mCluster.Cluster().method

uint32_t Instance::GetNumberOfSupportedAreas()
{
    FORWARD_TO_CLUSTER(GetNumberOfSupportedAreas());
}

bool Instance::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea)
{
    FORWARD_TO_CLUSTER(GetSupportedAreaByIndex(listIndex, aSupportedArea));
}

bool Instance::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
{
    FORWARD_TO_CLUSTER(GetSupportedAreaById(aAreaId, listIndex, aSupportedArea));
}

bool Instance::AddSupportedArea(AreaStructureWrapper & aNewArea)
{
    FORWARD_TO_CLUSTER(AddSupportedArea(aNewArea));
}

bool Instance::ModifySupportedArea(AreaStructureWrapper & aNewArea)
{
    FORWARD_TO_CLUSTER(ModifySupportedArea(aNewArea));
}

bool Instance::ClearSupportedAreas()
{
    FORWARD_TO_CLUSTER(ClearSupportedAreas());
}

bool Instance::RemoveSupportedArea(uint32_t areaId)
{
    FORWARD_TO_CLUSTER(RemoveSupportedArea(areaId));
}

uint32_t Instance::GetNumberOfSupportedMaps()
{
    FORWARD_TO_CLUSTER(GetNumberOfSupportedMaps());
}

bool Instance::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    FORWARD_TO_CLUSTER(GetSupportedMapByIndex(listIndex, aSupportedMap));
}

bool Instance::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    FORWARD_TO_CLUSTER(GetSupportedMapById(aMapId, listIndex, aSupportedMap));
}

bool Instance::AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName)
{
    FORWARD_TO_CLUSTER(AddSupportedMap(aMapId, aMapName));
}

bool Instance::RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName)
{
    FORWARD_TO_CLUSTER(RenameSupportedMap(aMapId, newMapName));
}

bool Instance::ClearSupportedMaps()
{
    FORWARD_TO_CLUSTER(ClearSupportedMaps());
}

bool Instance::RemoveSupportedMap(uint32_t mapId)
{
    FORWARD_TO_CLUSTER(RemoveSupportedMap(mapId));
}

uint32_t Instance::GetNumberOfSelectedAreas()
{
    FORWARD_TO_CLUSTER(GetNumberOfSelectedAreas());
}

bool Instance::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    FORWARD_TO_CLUSTER(GetSelectedAreaByIndex(listIndex, selectedArea));
}

bool Instance::AddSelectedArea(uint32_t & aSelectedArea)
{
    FORWARD_TO_CLUSTER(AddSelectedArea(aSelectedArea));
}

bool Instance::ClearSelectedAreas()
{
    FORWARD_TO_CLUSTER(ClearSelectedAreas());
}

bool Instance::RemoveSelectedAreas(uint32_t areaId)
{
    FORWARD_TO_CLUSTER(RemoveSelectedAreas(areaId));
}

DataModel::Nullable<uint32_t> Instance::GetCurrentArea()
{
    FORWARD_TO_CLUSTER(GetCurrentArea());
}

bool Instance::SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea)
{
    FORWARD_TO_CLUSTER(SetCurrentArea(aCurrentArea));
}

DataModel::Nullable<uint32_t> Instance::GetEstimatedEndTime()
{
    FORWARD_TO_CLUSTER(GetEstimatedEndTime());
}

bool Instance::SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    FORWARD_TO_CLUSTER(SetEstimatedEndTime(aEstimatedEndTime));
}

uint32_t Instance::GetNumberOfProgressElements()
{
    FORWARD_TO_CLUSTER(GetNumberOfProgressElements());
}

bool Instance::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    FORWARD_TO_CLUSTER(GetProgressElementByIndex(listIndex, aProgressElement));
}

bool Instance::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    FORWARD_TO_CLUSTER(GetProgressElementById(aAreaId, listIndex, aProgressElement));
}

bool Instance::AddPendingProgressElement(uint32_t aAreaId)
{
    FORWARD_TO_CLUSTER(AddPendingProgressElement(aAreaId));
}

bool Instance::SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus)
{
    FORWARD_TO_CLUSTER(SetProgressStatus(aAreaId, opStatus));
}

bool Instance::SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    FORWARD_TO_CLUSTER(SetProgressTotalOperationalTime(aAreaId, aTotalOperationalTime));
}

bool Instance::SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    FORWARD_TO_CLUSTER(SetProgressEstimatedTime(aAreaId, aEstimatedTime));
}

bool Instance::ClearProgress()
{
    FORWARD_TO_CLUSTER(ClearProgress());
}

bool Instance::RemoveProgressElement(uint32_t areaId)
{
    FORWARD_TO_CLUSTER(RemoveProgressElement(areaId));
}

bool Instance::HasFeature(ServiceArea::Feature feature) const
{
    return mCluster.Cluster().HasFeature(feature);
}

#undef FORWARD_TO_CLUSTER

// ZAP-generated plugin callbacks are left as stubs. Applications instantiate Instance directly
// (not through these callbacks) and register it with the codegen data model provider via Init().
void MatterServiceAreaClusterInitCallback(EndpointId endpointId) {}
void MatterServiceAreaClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType) {}
