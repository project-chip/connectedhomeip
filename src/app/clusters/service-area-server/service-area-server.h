/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>

#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

/**
 * Instance is a class that represents an instance of the generic Service Area cluster.
 * It implements AttributeAccessInterface and CommandHandlerInterface so it can
 * handle commands for any implementation of the location cluster.
 * Custom implementations of the Service Area cluster override functions in the Delegate class
 * must be provided to operate with specific devices.
 */
class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief Creates a Service Area cluster instance. The Init() method needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param[in] aDelegate A pointer to the delegate to be used by this server.
     * @param[in] aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param[in] aFeature The supported features of this Service Area Cluster.
     *
     * @note the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    Instance(StorageDelegate * storageDelegate, Delegate * aDelegate, EndpointId aEndpointId,
             BitMask<ServiceArea::Feature> aFeature);

    ~Instance() override;

    /**
     * Stop this class objects from being copied.
     */
    Instance(const Instance &)             = delete;
    Instance & operator=(const Instance &) = delete;

    /**
     * @brief Initialise the Service Area server instance.
     * @return CHIP_NO_ERROR if there are on errors. Returns an error if
     *   - the given endpoint and cluster ID have not been enabled in zap
     *   - if the CommandHandler or AttributeHandler registration fails
     *   - if the StorageDelegate or Delegate initialisation fails.
     */
    CHIP_ERROR Init();

private:
    StorageDelegate * mStorageDelegate;
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;

    // Attribute Data Store
    DataModel::Nullable<uint32_t> mCurrentArea;
    DataModel::Nullable<uint32_t> mEstimatedEndTime;
    BitMask<ServiceArea::Feature> mFeature;

    //*************************************************************************
    // core functions

    /**
     * @brief Read Attribute - inherited from AttributeAccessInterface.
     * @return appropriately mapped CHIP_ERROR if applicable.
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * @brief Command handler - inherited from CommandHandlerInterface.
     * @param[in, out] ctx command context.
     */
    void InvokeCommand(HandlerContext & ctx) override;

    //*************************************************************************
    // attribute readers

    CHIP_ERROR ReadSupportedAreas(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadSelectedAreas(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadProgress(chip::app::AttributeValueEncoder & aEncoder);

    //*************************************************************************
    // command handlers

    /**
     * @param[in, out] ctx Returns the Interaction Model status code which was user determined in the business logic.
     *                     If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * @param[in] req the command parameters.
     */
    void HandleSelectAreasCmd(HandlerContext & ctx, const Commands::SelectAreas::DecodableType & req);

    /**
     * @param[in, out] ctx Returns the Interaction Model status code which was user determined in the business logic.
     *                     If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * @param[in] req the command parameters.
     */
    void HandleSkipAreaCmd(HandlerContext & ctx, const Commands::SkipArea::DecodableType & req);

    //*************************************************************************
    // attribute notifications

    void NotifySupportedAreasChanged();
    void NotifySupportedMapsChanged();
    void NotifySelectedAreasChanged();
    void NotifyCurrentAreaChanged();
    void NotifyEstimatedEndTimeChanged();
    void NotifyProgressChanged();

    //*************************************************************************
    // Supported Areas helpers

    /**
     * @brief Check if the given area adheres to the restrictions required by the supported areas attribute.
     * @return true if the aArea meets all checks.
     */
    bool IsValidSupportedArea(const AreaStructureWrapper & aArea);

    /**
     * @brief check if aArea is unique with regard to supported areas.
     * @param[in] aArea the area to check.
     * @param[out] ignoreAreaId if true, we do not check if the area ID is unique.
     * @return true if there isn't an area in supported areas that matches aArea.
     *
     * @note This method may ignore checking the MapId uniqueness. This depends on whether the SupportedMaps attribute is null.
     */
    bool IsUniqueSupportedArea(const AreaStructureWrapper & aArea, bool ignoreAreaId);

    /**
     * @brief Check if changing the estimated end time attribute to aEstimatedEndTime requires the change to be reported.
     * @param aEstimatedEndTime The new estimated end time.
     * @return true if the change requires a report.
     */
    bool ReportEstimatedEndTimeChange(const DataModel::Nullable<uint32_t> & aEstimatedEndTime);

    /**
     * This method will ensure that the values in the Selected Areas, Current Area and Progress attributes correspond to areas in
     * the Supported Areas attribute.
     * Any invalid area IDs in the Selected Areas attribute will be removed.
     * If the Current Area is not in the Selected Areas attribute, it will be set to null.
     * Any progres elements with area IDs not in the Selected Areas attribute will be removed.
     */
    virtual void HandleSupportedAreasUpdated();

public:
    //*************************************************************************
    // Supported Areas accessors and manipulators

    uint32_t GetNumberOfSupportedAreas();

    /**
     * @brief Get a supported area using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aSupportedArea a copy of the area contents, if found.
     * @return true if an area is found, false otherwise.
     */
    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea);

    /**
     * @brief Get a supported area that matches a areaID.
     * @param[in] aAreaId the areaID to search for.
     * @param[out] listIndex the area's index in the list, if found.
     * @param[out] aSupportedArea a copy of the area contents, if found.
     * @return true if an area is found, false otherwise.
     */
    bool GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea);

    /**
     * @brief Add new location to the supported locations list.
     * @param[in] aNewArea The area to add.
     * @return true if the new location passed validation checks and was successfully added to the list.
     *
     * @note if aNewArea is larger than kAreaNameMaxSize, it will be truncated.
     */
    bool AddSupportedArea(AreaStructureWrapper & aNewArea);

    /**
     * @brief Modify/replace an existing location in the supported locations list.
     * @param[in] aNewArea The area to add.
     * @return true if the location is a member of supported locations, the modifications pass all validation checks and the
     * location was modified.
     *
     * @note if aNewArea is larger than kAreaNameMaxSize, it will be truncated.
     * @note if mapID is changed, the delegate's HandleSupportedAreasUpdated method is called.
     */
    bool ModifySupportedArea(AreaStructureWrapper & aNewArea);

    /**
     * @return true if the SupportedAreas attribute was not already null.
     *
     * @note if SupportedAreas is cleared, the delegate's HandleSupportedAreasUpdated method is called.
     */
    bool ClearSupportedAreas();

    /**
     * Removes the supported area with areaId.
     * If a supported area is removed, the Delegate's HandleSupportedAreasUpdated method is called to ensure that the
     * SelectedAreas, CurrentArea, and Progress attributes remain valid.
     * @param areaId The ID of the area to be removed.
     * @return true if an area with the areaId was removed. False otherwise.
     */
    bool RemoveSupportedArea(uint32_t areaId);

    //*************************************************************************
    // Supported Maps accessors and manipulators

    uint32_t GetNumberOfSupportedMaps();

    /**
     * @brief Get a supported map using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aSupportedMap  copy of the map contents, if found.
     * @return true if a supported map is found, false otherwise.
     */
    bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap);

    /**
     * @brief Get a supported map that matches a mapID.
     * @param[in] aMapId the mapID to search for.
     * @param[out] listIndex the map's index in the list, if found.
     * @param[out] aSupportedMap copy of the location contents, if found.
     * @return true if a supported map is found, false otherwise.
     */
    bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap);

    /**
     * @brief Add a new map to the supported maps list.
     * @param[in] aMapId The ID of the new map to be added.
     * @param[in] aMapName The name of the map to be added. This cannot be an empty string.
     * @return true if the new map passed validation checks and was successfully added to the list.
     */
    bool AddSupportedMap(uint32_t aMapId, const CharSpan & aMapName);

    /**
     * @brief Rename an existing map in the supported maps list.
     * @param[in] aMapId The id of the map to modify.
     * @param[in] newMapName The new name of the map. This cannot be empty string.
     * @return true if the new name passed validation checks and was successfully modified.
     *
     * @note if the specified map is not a member of the supported maps list, returns false with no action taken.
     */
    bool RenameSupportedMap(uint32_t aMapId, const CharSpan & newMapName);

    /**
     * @return true if the SupportedMaps attribute was not already null.
     *
     * @note if SupportedMaps is cleared, the delegate's HandleSupportedAreasUpdated method is called.
     */
    bool ClearSupportedMaps();

    /**
     * Removes the supported map with mapId.
     * If a supported map is removed, any supported areas that are no longer valid will also be removed.
     * @param mapId the ID of the map to be removed.
     * @return true if a map is removed. False otherwise.
     */
    bool RemoveSupportedMap(uint32_t mapId);

    //*************************************************************************
    // Selected Areas accessors and manipulators

    uint32_t GetNumberOfSelectedAreas();

    /**
     * @brief Get a selected area using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] selectedArea the selected area value, if found.
     * @return true if a selected area is found, false otherwise.
     */
    bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea);

    /**
     * @brief Add a selected area.
     * @param[in] aSelectedArea The areaID to add.
     * @bool true if successfully added.
     */
    bool AddSelectedArea(uint32_t & aSelectedArea);

    /**
     * @return true if the SelectedAreas attribute was not already null.
     */
    bool ClearSelectedAreas();

    /**
     * @param areaId the area ID to be removed from the SelectedAreas attribute.
     * @return ture if this ID was removed, false otherwise.
     */
    bool RemoveSelectedAreas(uint32_t areaId);

    //*************************************************************************
    // Current Area accessors and manipulators

    DataModel::Nullable<uint32_t> GetCurrentArea();

    /**
     * @param[in] aCurrentArea The area ID that the CurrentArea attribute should be set to. Must be a supported location
     * or NULL.
     * @return true if the current location is set, false otherwise.
     *
     * @note if current location is set to null, estimated end time will be set to null.
     */
    bool SetCurrentArea(const DataModel::Nullable<uint32_t> & aCurrentArea);

    //*************************************************************************
    // Estimated End Time accessors and manipulators

    /**
     * @return The estimated epoch time in seconds when operation at the location indicated by the CurrentArea attribute will be
     * completed.
     */
    DataModel::Nullable<uint32_t> GetEstimatedEndTime();

    /**
     * @param[in] aEstimatedEndTime The estimated epoch time in seconds when operation at the location indicated by the
     * CurrentArea attribute will be completed.
     * @return true if attribute is set, false otherwise.
     */
    bool SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime);

    //*************************************************************************
    // Progress list accessors and manipulators

    uint32_t GetNumberOfProgressElements();

    /**
     * @brief Get a progress element using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     */
    bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement);

    /**
     * @brief Get a progress element that matches a areaID.
     * @param[in] aAreaId the areaID to search for.
     * @param[out] listIndex the location's index in the list, if found.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     */
    bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement);

    /**
     * @brief Add a progress element in a pending status to the progress list.
     * @param[in] aAreaId location id of the progress element.
     * @return true if the new progress element passed validation checks and was successfully added to the list, false otherwise.
     */
    bool AddPendingProgressElement(uint32_t aAreaId);

    /**
     * @brief Set the status of progress element identified by areaID.
     * @param[in] aAreaId The areaID of the progress element to update.
     * @param[in] status The location cluster operation status for this location.
     * @return true if progress element is found and status is set, false otherwise.
     *
     * @note TotalOperationalTime is set to null if resulting opStatus is not equal to Completed or Skipped.
     */
    bool SetProgressStatus(uint32_t aAreaId, OperationalStatusEnum opStatus);

    /**
     * @brief Set the total operational time for the progress element identified by areaID.
     * @param[in] aAreaId The areaID of the progress element to update.
     * @param[in] aTotalOperationalTime The total operational time for this location.
     * @return true if progress element is found and operational time is set, false otherwise.
     */
    bool SetProgressTotalOperationalTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime);

    /**
     * @brief Set the estimated time for the  progress element identified by areaID.
     * @param[in] aAreaId The areaID of the progress element to update.
     * @param[in] aEstimatedTime The estimated time for this location.
     * @return true if progress element is found and estimated time is set, false otherwise.
     */
    bool SetProgressEstimatedTime(uint32_t aAreaId, const DataModel::Nullable<uint32_t> & aEstimatedTime);

    /**
     * @return true if the progress list was not already null, false otherwise.
     */
    bool ClearProgress();

    /**
     * @param areaId the area ID of the progress element to be removed.
     * @return ture if the progress element was removed, false otherwise.
     */
    bool RemoveProgressElement(uint32_t areaId);

    //*************************************************************************
    // Feature Map attribute

    /**
     * @brief Check if a feature is supported.
     * @param[in] feature the feature enum.
     * @return true if the feature is supported.
     *
     * @note the Service Area features are set at startup and are read-only to both device and client.
     */
    bool HasFeature(ServiceArea::Feature feature) const;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
