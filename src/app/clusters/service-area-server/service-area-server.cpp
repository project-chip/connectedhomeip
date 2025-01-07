/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#include "service-area-server.h"
#include "service-area-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using Status = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

// ****************************************************************************
// Service Area Server Instance

Instance::Instance(StorageDelegate * storageDelegate, Delegate * aDelegate, EndpointId aEndpointId, BitMask<Feature> aFeature) :
    AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
    mStorageDelegate(storageDelegate), mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(Id), mFeature(aFeature)
{
    ChipLogProgress(Zcl, "Service Area: Instance constructor");
    mDelegate->SetInstance(this);
}

Instance::~Instance()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::Init()
{
    ChipLogProgress(Zcl, "Service Area: INIT");

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(emberAfContainsServer(mEndpointId, Id), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "Service Area: The cluster with Id %lu was not enabled in zap.", long(Id)));

    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mStorageDelegate->Init());

    return mDelegate->Init();
}

//*************************************************************************
// core functions

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogDetail(Zcl, "Service Area: Reading attribute with ID " ChipLogFormatMEI, ChipLogValueMEI(aPath.mAttributeId));

    switch (aPath.mAttributeId)
    {

    case Attributes::SupportedAreas::Id:
        return ReadSupportedAreas(aEncoder);

    case Attributes::SupportedMaps::Id:
        return ReadSupportedMaps(aEncoder);

    case Attributes::SelectedAreas::Id:
        return ReadSelectedAreas(aEncoder);

    case Attributes::CurrentArea::Id:
        return aEncoder.Encode(GetCurrentArea());

    case Attributes::EstimatedEndTime::Id:
        return aEncoder.Encode(GetEstimatedEndTime());

    case Attributes::Progress::Id:
        return ReadProgress(aEncoder);

    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeature);

    default:
        ChipLogProgress(Zcl, "Service Area: Unsupported attribute with ID " ChipLogFormatMEI, ChipLogValueMEI(aPath.mAttributeId));
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SelectAreas::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SelectAreas::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSelectAreasCmd(ctx, req); });

    case Commands::SkipArea::Id:
        return CommandHandlerInterface::HandleCommand<Commands::SkipArea::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSkipAreaCmd(ctx, req); });
    }
}

//*************************************************************************
// attribute readers

CHIP_ERROR Instance::ReadSupportedAreas(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSupportedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint8_t locationIndex = 0;
        AreaStructureWrapper supportedArea;

        while (GetSupportedAreaByIndex(locationIndex++, supportedArea))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedArea));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSupportedMaps(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSupportedMaps() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t mapIndex = 0;
        MapStructureWrapper supportedMap;

        while (GetSupportedMapByIndex(mapIndex++, supportedMap))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedMap));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadSelectedAreas(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfSelectedAreas() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        uint32_t selectedArea;

        while (GetSelectedAreaByIndex(locationIndex++, selectedArea))
        {
            ReturnErrorOnFailure(encoder.Encode(selectedArea));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR Instance::ReadProgress(AttributeValueEncoder & aEncoder)
{
    if (GetNumberOfProgressElements() == 0)
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        uint32_t locationIndex = 0;
        Structs::ProgressStruct::Type progressElement;

        while (GetProgressElementByIndex(locationIndex++, progressElement))
        {
            ReturnErrorOnFailure(encoder.Encode(progressElement));
        }
        return CHIP_NO_ERROR;
    });
}

//*************************************************************************
// command handlers

void Instance::HandleSelectAreasCmd(HandlerContext & ctx, const Commands::SelectAreas::DecodableType & req)
{
    ChipLogDetail(Zcl, "Service Area: HandleSelectAreasCmd");

    // On receipt of this command the device SHALL respond with a SelectAreasResponse command.
    auto exitResponse = [ctx](SelectAreasStatus status, CharSpan statusText) {
        Commands::SelectAreasResponse::Type response{
            .status     = status,
            .statusText = statusText,
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    size_t numberOfAreas = 0;
    // Get the number of Selected Areas in the command parameter and check that it is valid.
    {
        if (CHIP_NO_ERROR != req.newAreas.ComputeSize(&numberOfAreas))
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }

        // If the device determines that it can't operate at all locations from the list,
        // the SelectAreasResponse command's Status field SHALL indicate InvalidSet.
        if (numberOfAreas > kMaxNumSelectedAreas)
        {
            exitResponse(SelectAreasStatus::kInvalidSet, "invalid number of locations"_span);
            return;
        }
    }

    uint32_t selectedAreasBuffer[kMaxNumSelectedAreas];
    auto selectedAreasSpan         = Span<uint32_t>(selectedAreasBuffer, kMaxNumSelectedAreas);
    uint32_t numberOfSelectedAreas = 0;

    // Closure for checking if an area ID exists in the selectedAreasSpan
    auto areaAlreadyExists = [&numberOfSelectedAreas, &selectedAreasSpan](uint32_t areaId) {
        for (uint32_t i = 0; i < numberOfSelectedAreas; i++)
        {
            if (areaId == selectedAreasSpan[i])
            {
                return true;
            }
        }
        return false;
    };

    // if number of selected locations in parameter matches number in attribute - the locations *might* be the same
    bool matchesCurrentSelectedAreas = (numberOfAreas == GetNumberOfSelectedAreas());

    // do as much parameter validation as we can
    if (numberOfAreas != 0)
    {
        uint32_t ignoredIndex = 0;
        uint32_t oldSelectedArea;
        auto iAreaIter = req.newAreas.begin();
        while (iAreaIter.Next())
        {
            uint32_t selectedArea = iAreaIter.GetValue();

            // If aSelectedArea is already in selectedAreasSpan skip
            if (areaAlreadyExists(selectedArea))
            {
                continue;
            }

            // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
            // If the Status field is set to UnsupportedArea, the StatusText field SHALL be an empty string.
            if (!mStorageDelegate->IsSupportedArea(selectedArea))
            {
                exitResponse(SelectAreasStatus::kUnsupportedArea, ""_span);
                return;
            }

            // check to see if parameter list and attribute still match
            if (matchesCurrentSelectedAreas)
            {
                if (!GetSelectedAreaByIndex(ignoredIndex, oldSelectedArea) || (selectedArea != oldSelectedArea))
                {
                    matchesCurrentSelectedAreas = false;
                }
            }

            selectedAreasSpan[numberOfSelectedAreas] = selectedArea;
            numberOfSelectedAreas += 1;
        }

        // after iterating with Next through DecodableType - check for failure
        if (CHIP_NO_ERROR != iAreaIter.GetStatus())
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
    }

    selectedAreasSpan.reduce_size(numberOfSelectedAreas);

    // If the newAreas field is the same as the value of the SelectedAreas attribute
    // the SelectAreasResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human-readable string or include an empty string.
    if (matchesCurrentSelectedAreas)
    {
        exitResponse(SelectAreasStatus::kSuccess, ""_span);
        return;
    }

    char delegateStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan delegateStatusText(delegateStatusBuffer);

    // If the current state of the device doesn't allow for the locations to be selected,
    // the SelectAreasResponse command SHALL have the Status field set to InvalidInMode.
    // if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed,
    // given the current mode of the device, which may involve other clusters.
    // (note - locationStatusText to be filled out by delegated function for if return value is false)
    if (!mDelegate->IsSetSelectedAreasAllowed(delegateStatusText))
    {
        exitResponse(SelectAreasStatus::kInvalidInMode, delegateStatusText);
        return;
    }

    // Reset in case the delegate accidentally modified this string.
    delegateStatusText = MutableCharSpan(delegateStatusBuffer);

    // ask the device to handle SelectAreas Command
    // (note - locationStatusText to be filled out by delegated function for kInvalidInMode and InvalidSet)
    auto locationStatus = SelectAreasStatus::kSuccess;
    if (!mDelegate->IsValidSelectAreasSet(selectedAreasSpan, locationStatus, delegateStatusText))
    {
        exitResponse(locationStatus, delegateStatusText);
        return;
    }

    {
        // If the device successfully accepts the request, the server will attempt to operate at the location(s)
        // indicated by the entries of the newArea field, when requested to operate,
        // the SelectAreasResponse command SHALL have the Status field set to Success,
        // and the SelectedAreas attribute SHALL be set to the value of the newAreas field.
        mStorageDelegate->ClearSelectedAreasRaw();

        if (numberOfAreas != 0)
        {
            uint32_t ignored;
            for (uint32_t areaId : selectedAreasSpan)
            {
                mStorageDelegate->AddSelectedAreaRaw(areaId, ignored);
            }
        }

        NotifySelectedAreasChanged();
    }

    exitResponse(SelectAreasStatus::kSuccess, ""_span);
}

void Instance::HandleSkipAreaCmd(HandlerContext & ctx, const Commands::SkipArea::DecodableType & req)
{
    ChipLogDetail(Zcl, "Service Area: HandleSkipArea");

    // On receipt of this command the device SHALL respond with a SkipAreaResponse command.
    auto exitResponse = [ctx](SkipAreaStatus status, CharSpan statusText) {
        Commands::SkipAreaResponse::Type response{
            .status     = status,
            .statusText = statusText,
        };
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    };

    // If the SelectedAreas attribute is empty, the SkipAreaResponse command’s Status field SHALL indicate InvalidAreaList.
    if (GetNumberOfSelectedAreas() == 0)
    {
        exitResponse(SkipAreaStatus::kInvalidAreaList, ""_span);
        return;
    }

    // If the SkippedArea field does not match an entry in the SupportedAreas attribute, the SkipAreaResponse command’s Status field
    // SHALL indicate InvalidSkippedArea.
    if (!mStorageDelegate->IsSupportedArea(req.skippedArea))
    {
        ChipLogError(Zcl, "SkippedArea (%" PRIu32 ") is not in the SupportedAreas attribute.", req.skippedArea);
        exitResponse(SkipAreaStatus::kInvalidSkippedArea, ""_span);
        return;
    }

    // have the device attempt to skip
    // If the Status field is not set to Success, or InvalidAreaList, the StatusText field SHALL include a vendor defined error
    // description. InvalidInMode | The received request cannot be handled due to the current mode of the device. (skipStatusText to
    // be filled out by delegated function on failure.)
    char skipStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan skipStatusText(skipStatusBuffer);

    if (!mDelegate->HandleSkipArea(req.skippedArea, skipStatusText))
    {
        exitResponse(SkipAreaStatus::kInvalidInMode, skipStatusText);
        return;
    }

    exitResponse(SkipAreaStatus::kSuccess, ""_span);
}

//*************************************************************************
// attribute notifications

void Instance::NotifySupportedAreasChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedAreas::Id);
}

void Instance::NotifySupportedMapsChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SupportedMaps::Id);
}

void Instance::NotifySelectedAreasChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::SelectedAreas::Id);
}

void Instance::NotifyCurrentAreaChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CurrentArea::Id);
}

void Instance::NotifyEstimatedEndTimeChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::EstimatedEndTime::Id);
}

void Instance::NotifyProgressChanged()
{
    MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::Progress::Id);
}

// ****************************************************************************
//  Supported Areas manipulators

bool Instance::IsValidSupportedArea(const AreaStructureWrapper & aArea)
{
    // If the LocationInfo field is null, the LandmarkInfo field SHALL NOT be null.
    // If the LandmarkInfo field is null, the LocationInfo field SHALL NOT be null.
    if (aArea.areaInfo.locationInfo.IsNull() && aArea.areaInfo.landmarkInfo.IsNull())
    {
        ChipLogDetail(Zcl, "IsValidAsSupportedArea %" PRIu32 " - must have locationInfo and/or LandmarkInfo", aArea.areaID);
        return false;
    }

    // If LocationInfo is not null, and its LocationName field is an empty string, at least one of the following SHALL NOT
    // be null: LocationInfo's FloorNumber field, LocationInfo's AreaType field, the LandmarkInfo
    if (!aArea.areaInfo.locationInfo.IsNull())
    {
        if (aArea.areaInfo.locationInfo.Value().locationName.empty() && aArea.areaInfo.locationInfo.Value().floorNumber.IsNull() &&
            aArea.areaInfo.locationInfo.Value().areaType.IsNull() && aArea.areaInfo.landmarkInfo.IsNull())
        {
            ChipLogDetail(
                Zcl, "IsValidAsSupportedArea %" PRIu32 " - AreaName is empty string, FloorNumber, AreaType, LandmarkInfo are null",
                aArea.areaID);
            return false;
        }
    }

    // The mapID field SHALL be null if SupportedMaps is not supported or SupportedMaps is an empty list.
    if (mFeature.Has(Feature::kMaps) && (GetNumberOfSupportedMaps() > 0))
    {
        if (aArea.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id should not be null when there are supported maps",
                          aArea.areaID);
            return false;
        }

        // If the SupportedMaps attribute is not null, mapID SHALL be the ID of an entry from the SupportedMaps attribute.
        if (!mStorageDelegate->IsSupportedMap(aArea.mapID.Value()))
        {
            ChipLogError(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id %" PRIu32 " is not in supported map list", aArea.areaID,
                         aArea.mapID.Value());
            return false;
        }
    }
    else
    {
        if (!aArea.mapID.IsNull())
        {
            ChipLogDetail(Zcl, "IsValidSupportedArea %" PRIu32 " - map Id %" PRIu32 " is not in empty supported map list",
                          aArea.areaID, aArea.mapID.Value());
            return false;
        }
    }

    return true;
}

bool Instance::IsUniqueSupportedArea(const AreaStructureWrapper & aArea, bool ignoreAreaId)
{
    BitMask<AreaStructureWrapper::IsEqualConfig> config;

    if (ignoreAreaId)
    {
        config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreAreaID);
    }

    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and LocationInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the LocationInfo field.
    if (GetNumberOfSupportedMaps() == 0)
    {
        config.Set(AreaStructureWrapper::IsEqualConfig::kIgnoreMapId);
    }

    uint8_t locationIndex = 0;
    AreaStructureWrapper entry;
    while (GetSupportedAreaByIndex(locationIndex++, entry))
    {
        if (aArea.IsEqual(entry, config))
        {
            return false;
        }
    }

    return true;
}

bool Instance::ReportEstimatedEndTimeChange(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    if (mEstimatedEndTime == aEstimatedEndTime)
    {
        return false;
    }

    // The value of this attribute SHALL only be reported in the following cases:
    // - when it changes from null.
    if (mEstimatedEndTime.IsNull())
    {
        return true;
    }

    // - when it changes from 0
    if (mEstimatedEndTime.Value() == 0)
    {
        return true;
    }

    if (aEstimatedEndTime.IsNull())
    {
        return false;
    }

    // From this point we know that mEstimatedEndTime and aEstimatedEndTime are not null and not the same.

    // - when it changes to 0
    if (aEstimatedEndTime.Value() == 0)
    {
        return true;
    }

    // - when it decreases
    return (aEstimatedEndTime.Value() < mEstimatedEndTime.Value());
}

void Instance::HandleSupportedAreasUpdated()
{
    // If there are no more Supported Areas, clear all selected areas, current area, and progress.
    if (GetNumberOfSupportedAreas() == 0)
    {
        ClearSelectedAreas();
        SetCurrentArea(DataModel::NullNullable);
        ClearProgress();
        return;
    }

    // Remove Selected Areas elements that do not exist is the Supported Areas attribute.
    {
        uint32_t i                  = 0;
        uint32_t areaId             = 0;
        uint32_t areasToRemoveIndex = 0;
        uint32_t areasToRemoveBuffer[kMaxNumSelectedAreas];
        Span<uint32_t> areasToRemoveSpan(areasToRemoveBuffer);

        while (GetSelectedAreaByIndex(i, areaId))
        {
            if (!mStorageDelegate->IsSupportedArea(areaId))
            {
                areasToRemoveSpan[areasToRemoveIndex] = areaId;
                areasToRemoveIndex++;
            }
            i++;
        }
        areasToRemoveSpan.reduce_size(areasToRemoveIndex);

        for (auto id : areasToRemoveSpan)
        {
            if (!RemoveSelectedAreas(id))
            {
                ChipLogError(Zcl, "HandleSupportedAreasUpdated: Failed to remove area %" PRIu32 " from selected areas", id);
            }
        }
    }

    // Set current Area to null if current area is not in the supported areas attribute.
    {
        auto currentAreaId = GetCurrentArea();
        if (!currentAreaId.IsNull() && !mStorageDelegate->IsSupportedArea(currentAreaId.Value()))
        {
            SetCurrentArea(DataModel::NullNullable);
        }
    }

    // Remove Progress elements associated with areas that do not exist is the Supported Areas attribute.
    {
        uint32_t i = 0;
        Structs::ProgressStruct::Type tempProgressElement;
        uint32_t progressToRemoveIndex = 0;
        uint32_t progressToRemoveBuffer[kMaxNumProgressElements];
        Span<uint32_t> progressToRemoveSpan(progressToRemoveBuffer);

        while (mStorageDelegate->GetProgressElementByIndex(i, tempProgressElement))
        {
            if (mStorageDelegate->IsSupportedArea(tempProgressElement.areaID))
            {
                progressToRemoveSpan[progressToRemoveIndex] = tempProgressElement.areaID;
                progressToRemoveIndex++;
            }
            i++;
        }
        progressToRemoveSpan.reduce_size(progressToRemoveIndex);

        for (auto areaId : progressToRemoveSpan)
        {
            if (!mStorageDelegate->RemoveProgressElementRaw(areaId))
            {
                ChipLogError(Zcl, "HandleSupportedAreasUpdated: Failed to remove progress element with area ID %" PRIu32 "",
                             areaId);
            }
        }
    }
}

uint32_t Instance::GetNumberOfSupportedAreas()
{
    return mStorageDelegate->GetNumberOfSupportedAreas();
}

bool Instance::GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mStorageDelegate->GetSupportedAreaByIndex(listIndex, aSupportedArea);
}

bool Instance::GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea)
{
    return mStorageDelegate->GetSupportedAreaById(aAreaId, listIndex, aSupportedArea);
}

bool Instance::AddSupportedArea(AreaStructureWrapper & aNewArea)
{
    // Does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    // Check there is space for the entry.
    if (GetNumberOfSupportedAreas() >= kMaxNumSupportedAreas)
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - too many entries", aNewArea.areaID);
        return false;
    }

    // Verify cluster requirements concerning valid fields and field relationships.
    if (!IsValidSupportedArea(aNewArea))
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - not a valid location object", aNewArea.areaID);
        return false;
    }

    // Each entry in Supported Areas SHALL have a unique value for the ID field.
    // If the SupportedMaps attribute is not null, each entry in this list SHALL have a unique value for the combination of the
    // MapID and AreaInfo fields. If the SupportedMaps attribute is null, each entry in this list SHALL have a unique value for
    // the AreaInfo field.
    if (!IsUniqueSupportedArea(aNewArea, false))
    {
        ChipLogError(Zcl, "AddSupportedAreaRaw %" PRIu32 " - not a unique location object", aNewArea.areaID);
        return false;
    }

    // Add the SupportedArea to the SupportedAreas attribute.
    uint32_t ignoredIndex;
    if (!mStorageDelegate->AddSupportedAreaRaw(aNewArea, ignoredIndex))
    {
        return false;
    }

    NotifySupportedAreasChanged();
    return true;
}

bool Instance::ModifySupportedArea(AreaStructureWrapper & aNewArea)
{
    bool mapIDChanged = false;
    uint32_t listIndex;

    // get existing supported location to modify
    AreaStructureWrapper supportedArea;
    if (!GetSupportedAreaById(aNewArea.areaID, listIndex, supportedArea))
    {
        ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a supported areaID", aNewArea.areaID);
        return false;
    }

    {
        // check for mapID change
        if ((aNewArea.mapID.IsNull() != supportedArea.mapID.IsNull()) ||
            (!aNewArea.mapID.IsNull() && !supportedArea.mapID.IsNull() && (aNewArea.mapID.Value() != supportedArea.mapID.Value())))
        {
            // does device mode allow this attribute to be updated?
            if (!mDelegate->IsSupportedAreasChangeAllowed())
            {
                return false;
            }
            mapIDChanged = true;
        }

        // verify cluster requirements concerning valid fields and field relationships
        if (!IsValidSupportedArea(aNewArea))
        {
            ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a valid location object", aNewArea.areaID);
            return false;
        }

        // Updated location description must not match another existing location description.
        // We ignore comparing the area ID as one of the locations will match this one.
        if (!IsUniqueSupportedArea(aNewArea, true))
        {
            ChipLogError(Zcl, "ModifySupportedAreaRaw %" PRIu32 " - not a unique location object", aNewArea.areaID);
            return false;
        }

        // Replace the supported location with the modified location.
        if (!mStorageDelegate->ModifySupportedAreaRaw(listIndex, aNewArea))
        {
            return false;
        }
    }

    if (mapIDChanged)
    {
        HandleSupportedAreasUpdated();
    }

    NotifySupportedAreasChanged();
    return true;
}

bool Instance::ClearSupportedAreas()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedAreasChangeAllowed())
    {
        return false;
    }

    if (mStorageDelegate->ClearSupportedAreasRaw())
    {
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
        return true;
    }

    return false;
}

bool Instance::RemoveSupportedArea(uint32_t areaId)
{
    if (mStorageDelegate->RemoveSupportedAreaRaw(areaId))
    {
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
        return true;
    }
    return false;
}

//*************************************************************************
// Supported Maps manipulators

uint32_t Instance::GetNumberOfSupportedMaps()
{
    return mStorageDelegate->GetNumberOfSupportedMaps();
}

bool Instance::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap)
{
    return mStorageDelegate->GetSupportedMapByIndex(listIndex, aSupportedMap);
}

bool Instance::GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    return mStorageDelegate->GetSupportedMapById(aMapId, listIndex, aSupportedMap);
}

bool Instance::AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName)
{
    // check max# of list entries
    if (GetNumberOfSupportedMaps() >= kMaxNumSupportedMaps)
    {
        ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - maximum number of entries", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map name (cannot be empty string).
    if (aMapName.empty())
    {
        ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - Name must not be empty string", aMapId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the Name field.
    uint8_t mapIndex = 0;
    MapStructureWrapper entry;

    while (GetSupportedMapByIndex(mapIndex++, entry))
    {
        // the name cannot be the same as an existing map
        if (entry.IsNameEqual(aMapName))
        {
            ChipLogError(Zcl, "AddSupportedMapRaw %" PRIu32 " - A map already exists with same name '%.*s'", aMapId,
                         static_cast<int>(entry.GetName().size()), entry.GetName().data());
            return false;
        }

        //  Each entry in this list SHALL have a unique value for the MapID field.
        if (aMapId == entry.mapID)
        {
            ChipLogError(Zcl, "AddSupportedMapRaw - non-unique Id %" PRIu32 "", aMapId);
            return false;
        }
    }

    {
        // add to supported maps attribute
        MapStructureWrapper newMap(aMapId, aMapName);
        uint32_t ignoredIndex;
        if (!mStorageDelegate->AddSupportedMapRaw(newMap, ignoredIndex))
        {
            return false;
        }
    }

    // map successfully added
    NotifySupportedMapsChanged();
    return true;
}

bool Instance::RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName)
{
    uint32_t modifiedIndex;
    MapStructureWrapper modifiedMap;

    // get existing entry
    if (!GetSupportedMapById(aMapId, modifiedIndex, modifiedMap))
    {
        ChipLogError(Zcl, "RenameSupportedMap Id %" PRIu32 " - map does not exist", aMapId);
        return false;
    }

    //  Map name SHALL include readable text that describes the map's name. It cannot be empty string.
    if (newMapName.empty())
    {
        ChipLogError(Zcl, "RenameSupportedMap %" PRIu32 " - Name must not be empty string", aMapId);
        return false;
    }

    // update the local copy of the map
    modifiedMap.Set(modifiedMap.mapID, newMapName);

    // Each entry in this list SHALL have a unique value for the Name field.
    uint32_t loopIndex = 0;
    MapStructureWrapper entry;

    while (GetSupportedMapByIndex(loopIndex, entry))
    {
        if (modifiedIndex == loopIndex)
        {
            continue; // don't check local modified map against its own list entry
        }

        if (entry.IsNameEqual(newMapName))
        {
            ChipLogError(Zcl, "RenameSupportedMap %" PRIu32 " - map already exists with same name '%.*s'", aMapId,
                         static_cast<int>(entry.GetName().size()), entry.GetName().data());
            return false;
        }

        ++loopIndex;
    }

    if (!mStorageDelegate->ModifySupportedMapRaw(modifiedIndex, modifiedMap))
    {
        return false;
    }

    // map successfully renamed
    NotifySupportedMapsChanged();
    return true;
}

bool Instance::ClearSupportedMaps()
{
    // does device mode allow this attribute to be updated?
    if (!mDelegate->IsSupportedMapChangeAllowed())
    {
        return false;
    }

    if (mStorageDelegate->ClearSupportedMapsRaw())
    {
        ClearSupportedAreas();
        NotifySupportedMapsChanged();
        return true;
    }

    return false;
}

bool Instance::RemoveSupportedMap(uint32_t mapId)
{
    if (!mStorageDelegate->RemoveSupportedMapRaw(mapId))
    {
        return false;
    }

    NotifySupportedMapsChanged();

    // If there are no supported maps left, none of the supported areas are vaild and their MapID needs to be null.
    if (GetNumberOfSupportedMaps() == 0)
    {
        ClearSupportedAreas();
        return true;
    }

    // Get the supported area IDs where the map ID matches the removed map ID.
    uint32_t supportedAreaIdsBuffer[kMaxNumSupportedAreas];
    Span<uint32_t> supportedAreaIdsSpan(supportedAreaIdsBuffer);
    {
        uint32_t supportedAreaIdsSize = 0;
        uint32_t supportedAreasIndex  = 0;
        AreaStructureWrapper tempSupportedArea;
        while (mStorageDelegate->GetSupportedAreaByIndex(supportedAreasIndex, tempSupportedArea))
        {
            if (tempSupportedArea.mapID == mapId)
            {
                supportedAreaIdsSpan[supportedAreasIndex] = tempSupportedArea.areaID;
                supportedAreaIdsSize++;
            }
            supportedAreasIndex++;
        }

        supportedAreaIdsSpan.reduce_size(supportedAreaIdsSize);
    }

    if (!supportedAreaIdsSpan.empty())
    {
        for (uint32_t supportedAreaId : supportedAreaIdsSpan)
        {
            mStorageDelegate->RemoveSupportedAreaRaw(supportedAreaId);
        }
        HandleSupportedAreasUpdated();
        NotifySupportedAreasChanged();
    }

    return true;
}

//*************************************************************************
// Selected Areas manipulators

uint32_t Instance::GetNumberOfSelectedAreas()
{
    return mStorageDelegate->GetNumberOfSelectedAreas();
}

bool Instance::GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea)
{
    return mStorageDelegate->GetSelectedAreaByIndex(listIndex, selectedArea);
}

bool Instance::AddSelectedArea(uint32_t & aSelectedArea)
{
    // check max# of list entries
    if (GetNumberOfSelectedAreas() >= kMaxNumSelectedAreas)
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - maximum number of entries", aSelectedArea);
        return false;
    }

    // each item in this list SHALL match the AreaID field of an entry on the SupportedAreas attribute's list
    if (!mStorageDelegate->IsSupportedArea(aSelectedArea))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - not a supported location", aSelectedArea);
        return false;
    }

    // each entry in this list SHALL have a unique value
    if (mStorageDelegate->IsSelectedArea(aSelectedArea))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - duplicated location", aSelectedArea);
        return false;
    }

    // Does device mode allow modification of selected locations?
    char locationStatusBuffer[kMaxSizeStatusText];
    MutableCharSpan locationStatusText(locationStatusBuffer);

    if (!mDelegate->IsSetSelectedAreasAllowed(locationStatusText))
    {
        ChipLogError(Zcl, "AddSelectedAreaRaw %" PRIu32 " - %.*s", aSelectedArea, static_cast<int>(locationStatusText.size()),
                     locationStatusText.data());
        return false;
    }

    uint32_t ignoredIndex;
    return mStorageDelegate->AddSelectedAreaRaw(aSelectedArea, ignoredIndex);
}

bool Instance::ClearSelectedAreas()
{
    if (mStorageDelegate->ClearSelectedAreasRaw())
    {
        NotifySelectedAreasChanged();
        return true;
    }

    return false;
}

bool Instance::RemoveSelectedAreas(uint32_t areaId)
{
    if (mStorageDelegate->RemoveSelectedAreasRaw(areaId))
    {
        NotifySelectedAreasChanged();
        return true;
    }

    return false;
}

//*************************************************************************
// Current Area manipulators

DataModel::Nullable<uint32_t> Instance::GetCurrentArea()
{
    return mCurrentArea;
}

bool Instance::SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea)
{
    // If not null, the value of this attribute SHALL match the AreaID field of an entry on the SupportedAreas attribute's
    // list.
    if ((!aCurrentArea.IsNull()) && (!mStorageDelegate->IsSupportedArea(aCurrentArea.Value())))
    {
        ChipLogError(Zcl, "SetCurrentArea %" PRIu32 " - location is not supported", aCurrentArea.Value());
        return false;
    }

    bool notifyChange = mCurrentArea != aCurrentArea;

    mCurrentArea = aCurrentArea;
    if (notifyChange)
    {
        NotifyCurrentAreaChanged();
    }

    // EstimatedEndTime SHALL be null if the CurrentArea attribute is null.
    if (mCurrentArea.IsNull())
    {
        SetEstimatedEndTime(DataModel::NullNullable);
    }

    return true;
}

//*************************************************************************
// Estimated End Time manipulators

DataModel::Nullable<uint32_t> Instance::GetEstimatedEndTime()
{
    return mEstimatedEndTime;
}

bool Instance::SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime)
{
    // EstimatedEndTime SHALL be null if the CurrentArea attribute is null.
    if (mCurrentArea.IsNull() && !aEstimatedEndTime.IsNull())
    {
        ChipLogError(Zcl, "SetEstimatedEndTime - must be null if Current Area is null");
        return false;
    }

    bool notifyChange = ReportEstimatedEndTimeChange(aEstimatedEndTime);

    mEstimatedEndTime = aEstimatedEndTime;

    if (notifyChange)
    {
        NotifyEstimatedEndTimeChanged();
    }

    // success
    return true;
}

//*************************************************************************
// Progress list manipulators

uint32_t Instance::GetNumberOfProgressElements()
{
    return mStorageDelegate->GetNumberOfProgressElements();
}

bool Instance::GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    return mStorageDelegate->GetProgressElementByIndex(listIndex, aProgressElement);
}

bool Instance::GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    return mStorageDelegate->GetProgressElementById(aAreaId, listIndex, aProgressElement);
}

bool Instance::AddPendingProgressElement(uint32_t aAreaId)
{
    // create progress element
    Structs::ProgressStruct::Type inactiveProgress = { aAreaId, OperationalStatusEnum::kPending };

    // check max# of list entries
    if (GetNumberOfProgressElements() >= kMaxNumProgressElements)
    {
        ChipLogError(Zcl, "AddPendingProgressElement - maximum number of entries");
        return false;
    }

    // For each entry in this list, the AreaID field SHALL match an entry on the SupportedAreas attribute's list.
    if (!mStorageDelegate->IsSupportedArea(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - not a supported location %" PRIu32 "", aAreaId);
        return false;
    }

    // Each entry in this list SHALL have a unique value for the AreaID field.
    if (mStorageDelegate->IsProgressElement(aAreaId))
    {
        ChipLogError(Zcl, "AddPendingProgressElement - progress element already exists for location %" PRIu32 "", aAreaId);
        return false;
    }

    uint32_t ignoredIndex;

    if (!mStorageDelegate->AddProgressElementRaw(inactiveProgress, ignoredIndex))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressStatus - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the status value is not changing, there in no need to modify the existing element.
    if (progressElement.status == opStatus)
    {
        return true;
    }

    // set the progress status in the local copy
    progressElement.status = opStatus;

    // TotalOperationalTime SHALL be null if the Status field is not set to Completed or Skipped.
    if ((opStatus != OperationalStatusEnum::kCompleted) && (opStatus != OperationalStatusEnum::kSkipped))
    {
        progressElement.totalOperationalTime.Emplace(DataModel::NullNullable);
    }

    // add the updated element to the progress attribute
    if (!mStorageDelegate->ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressTotalOperationalTime - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.totalOperationalTime == aTotalOperationalTime)
    {
        return true;
    }

    // This attribute SHALL be null if the Status field is not set to Completed or Skipped
    if (((progressElement.status == OperationalStatusEnum::kCompleted) ||
         (progressElement.status == OperationalStatusEnum::kSkipped)) &&
        !aTotalOperationalTime.IsNull())
    {
        ChipLogError(Zcl,
                     "SetProgressTotalOperationalTime - location %" PRIu32
                     " opStatus value %u - can be non-null only if opStatus is "
                     "Completed or Skipped",
                     aAreaId, to_underlying(progressElement.status));
        return false;
    }

    // set the time in the local copy
    progressElement.totalOperationalTime.Emplace(aTotalOperationalTime);

    // add the updated element to the progress attribute
    if (!mStorageDelegate->ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime)
{
    uint32_t listIndex;
    Structs::ProgressStruct::Type progressElement;

    if (!GetProgressElementById(aAreaId, listIndex, progressElement))
    {
        ChipLogError(Zcl, "SetProgressEstimatedTime - progress element does not exist for location %" PRIu32 "", aAreaId);
        return false;
    }

    // If the time value is not changing, there is no need to modify the existing element.
    if (progressElement.estimatedTime == aEstimatedTime)
    {
        return true;
    };

    // set the time in the local copy
    progressElement.estimatedTime.Emplace(aEstimatedTime);

    // add the updated element to the progress attribute
    if (!mStorageDelegate->ModifyProgressElementRaw(listIndex, progressElement))
    {
        return false;
    }

    NotifyProgressChanged();
    return true;
}

bool Instance::ClearProgress()
{
    if (mStorageDelegate->ClearProgressRaw())
    {
        NotifyProgressChanged();
        return true;
    }

    return false;
}

bool Instance::RemoveProgressElement(uint32_t areaId)
{
    if (mStorageDelegate->RemoveProgressElementRaw(areaId))
    {
        NotifyProgressChanged();
        return true;
    }

    return false;
}

// attribute manipulators - Feature Map

bool Instance::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
