/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ServiceArea/Attributes.h>
#include <clusters/ServiceArea/Metadata.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

/**
 * ServiceAreaCluster is the code-driven implementation of the Service Area cluster.
 *
 * It implements the cluster data model via DefaultServerCluster and delegates list storage
 * to StorageDelegate and device-specific business logic to Delegate. Applications that use
 * the legacy Instance wrapper (see CodegenIntegration.h) interact with this class indirectly.
 */
class ServiceAreaCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet =
        app::OptionalAttributeSet<ServiceArea::Attributes::SupportedMaps::Id, ServiceArea::Attributes::CurrentArea::Id,
                                  ServiceArea::Attributes::EstimatedEndTime::Id, ServiceArea::Attributes::Progress::Id>;

    /**
     * @param[in] endpointId The endpoint on which this cluster exists.
     * @param[in] storageDelegate Storage for list attributes. Must outlive this cluster.
     * @param[in] delegate Application business logic. Must outlive this cluster.
     * @param[in] feature The supported features of this Service Area cluster.
     * @param[in] optionalAttributes Optional attributes enabled in the endpoint configuration.
     */
    ServiceAreaCluster(EndpointId endpointId, StorageDelegate & storageDelegate, Delegate & delegate,
                       BitMask<ServiceArea::Feature> feature, OptionalAttributeSet optionalAttributes = {});

    ~ServiceAreaCluster() override = default;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

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
     * @note if mapID is changed, HandleSupportedAreasUpdated is called.
     */
    bool ModifySupportedArea(AreaStructureWrapper & aNewArea);

    /**
     * @return true if the SupportedAreas attribute was not already null.
     *
     * @note if SupportedAreas is cleared, HandleSupportedAreasUpdated is called.
     */
    bool ClearSupportedAreas();

    /**
     * Removes the supported area with areaId.
     * If a supported area is removed, HandleSupportedAreasUpdated is called to ensure that the
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
     * @note if SupportedMaps is cleared, HandleSupportedAreasUpdated is called.
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
     * @return true if successfully added.
     */
    bool AddSelectedArea(uint32_t & aSelectedArea);

    /**
     * @return true if the SelectedAreas attribute was not already null.
     */
    bool ClearSelectedAreas();

    /**
     * @param areaId the area ID to be removed from the SelectedAreas attribute.
     * @return true if this ID was removed, false otherwise.
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
     * @return true if the progress element was removed, false otherwise.
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

protected:
    /**
     * This method will ensure that the values in the Selected Areas, Current Area and Progress attributes correspond to areas in
     * the Supported Areas attribute.
     * Any invalid area IDs in the Selected Areas attribute will be removed.
     * If the Current Area is not in the Selected Areas attribute, it will be set to null.
     * Any progress elements with area IDs not in the Selected Areas attribute will be removed.
     */
    virtual void HandleSupportedAreasUpdated();

private:
    StorageDelegate & mStorageDelegate;
    Delegate & mDelegate;

    // Attribute Data Store
    DataModel::Nullable<uint32_t> mCurrentArea;
    DataModel::Nullable<uint32_t> mEstimatedEndTime;
    const BitMask<ServiceArea::Feature> mFeature;
    const OptionalAttributeSet mOptionalAttributes;

    bool SupportsSkipArea() const;

    //*************************************************************************
    // attribute readers

    CHIP_ERROR ReadSupportedAreas(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadSelectedAreas(chip::app::AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadProgress(chip::app::AttributeValueEncoder & aEncoder);

    //*************************************************************************
    // command handlers

    std::optional<DataModel::ActionReturnStatus> HandleSelectAreasCmd(const DataModel::InvokeRequest & request,
                                                                      const Commands::SelectAreas::DecodableType & req,
                                                                      CommandHandler * handler);

    std::optional<DataModel::ActionReturnStatus> HandleSkipAreaCmd(const DataModel::InvokeRequest & request,
                                                                   const Commands::SkipArea::DecodableType & req,
                                                                   CommandHandler * handler);

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
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
