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
class Instance : public AttributeAccessInterface, public CommandHandlerInterface, public Uncopyable
{
public:
    /**
     * @brief Creates a Service Area cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param[in] aDelegate A pointer to the delegate to be used by this server.
     * @param[in] aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param[in] aFeature The supported features of this Service Area Cluster
     *
     * @note the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, BitMask<ServiceArea::Feature> aFeature);

    /**
     * @brief Destructor
     */
    ~Instance() override;

    /**
     * @brief Initialise the Service Area server instance.
     * @return an error if the given endpoint and cluster Id have not been enabled in zap or if the
     *         CommandHandler or AttributeHandler registration fails, else CHIP_NO_ERROR.
     *         This method also checks if the feature setting is valid, if invalid return value will be CHIP_ERROR_INVALID_ARGUMENT.
     * 
     * @note This function must be called after defining an Instance class object.
     */
    CHIP_ERROR Init();


private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId  mClusterId;

    // Attribute Data Store
    DataModel::Nullable<uint32_t>                      mCurrentLocation;     // may be volatile - callback to device to refresh value is provided
    DataModel::Nullable<uint32_t>                      mEstimatedEndTime;    // may be volatile - callback to device to refresh value is provided
    BitMask<ServiceArea::Feature>                      mFeature;


    //*************************************************************************
    // core functions

    /**
     * @brief Read Attribute - inherited from AttributeAccessInterface
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * @brief Command handler - inherited from CommandHandlerInterface
     * @param[in, out] ctx command context
     */
    void InvokeCommand(HandlerContext & ctx) override;


    //*************************************************************************
    // attribute readers

    /**
     * @brief Read the Supported Locations Attribute
     * @param [in, out] aEncoder attribute encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSupportedLocations(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * @brief Read the Supported Maps Attribute
     * @param [in, out] aEncoder attribute encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * @brief Read the Selected Locations Attribute
     * @param [in, out] aEncoder attribute encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSelectedLocations(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * Read the Progress Attribute
     * @param [in, out] aEncoder attribute encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadProgress(chip::app::AttributeValueEncoder & aEncoder);


    //*************************************************************************
    // command handlers

    /**
     * @brief Handle Command: SelectLocations.
     * @param[in, out] ctx Returns the Interaction Model status code which was user determined in the business logic.
     *                     If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * @param[in] req the command parameters
     */
    void HandleSelectLocationsCmd(HandlerContext & ctx, const Commands::SelectLocations::DecodableType & req);
                            
    /**
     * @brief Handle Command: SkipCurrentLocation.
     * @param[in, out] ctx Returns the Interaction Model status code which was user determined in the business logic.
     *                     If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     */
    void HandleSkipCurrentLocationCmd(HandlerContext & ctx);

    //*************************************************************************
    // attribute notifications

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifySupportedLocationsChanged();

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifySupportedMapsChanged();

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifySelectedLocationsChanged();

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifyCurrentLocationChanged();

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifyEstimatedEndTimeChanged();

    /**
     * @brief Notify MATTER that the attribute has changed
     */
    void NotifyProgressChanged();

    //*************************************************************************
    // Supported Locations manipulators

    /**
     * @brief Is the location in the supported locations list?
     * @param[in] aLocationId ID of the location.
     * @return true if the location identified by Id is in the supported locations list
     * 
     * @note  use GetSupportedLocartionById if location contents are needed.
     */ 
    bool IsSupportedLocation(uint32_t aLocationId);

    /**
     * @brief check Location against cluster requirements for supported locations
     * @param[in] aLocation the location structure to be validated.
     * @return true if meets all checks
     */
    bool IsValidSupportedLocation(const LocationStructureWrapper & aLocation);

    /**
     * @brief check if aLocation is unique with regard to supported locations.
     * @param[in] aLocation the location to check.
     * @param[out] ignoreLocationId if true, we do not check if the location ID is unique.
     * @return true if there isn't a location in supported locations that matches aLocation.
     *
     * @note This method may ignore checking the MapId uniqueness. This depends on whether the SupportedMaps attribute is null.
     */
    bool IsUniqueSupportedLocation(const LocationStructureWrapper & aLocation, bool ignoreLocationId);


public:

    /**
     * @brief Add new location to the supported locations list
     * @param[in] aLocationId unique identifier of this location
     * @param[in] aMapId identifier of supported map
     * @param[in] aLocationName human readable name for this location (empty string if not used)
     * @param[in] aFloorNumber represents floor level - negative values for below ground
     * @param[in] aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param[in] aLandmarkTag common namespace Landmark tag - indicates an association of the location with a home landmark
     * @param[in] aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param[in] aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * @return true if the new location passed validation checks and was successfully added to the list
     * 
     * @note if aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note MATTER change notifications are made for the attributes that change.
     */
    bool AddSupportedLocation( uint32_t                                     aLocationId, 
                               const DataModel::Nullable<uint8_t>         & aMapId, 
                               const CharSpan                             & aLocationName,
                               const DataModel::Nullable<int16_t>         & aFloorNumber,
                               const DataModel::Nullable<AreaTypeTag>     & aAreaType,
                               const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                               const DataModel::Nullable<PositionTag>     & aPositionTag,
                               const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  );



    /**
     * @brief Modify/replace an existing location in the supported locations list
     * @param[in] aLocationId unique identifier of this location
     * @param[in] aMapId identifier of supported map (will not be modified)
     * @param[in] aLocationName human readable name for this location (empty string if not used)
     * @param[in] aFloorNumber represents floor level - negative values for below ground
     * @param[in] aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param[in] aLandmarkTag common namespace Landmak tag - indicates an association of the location with a home landmark
     * @param[in] aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param[in] aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * @return true if the location is a member of supported locations, the modifications pass all validation checks and the location was modified 
     *
     * @note if aLocationName is larger than kLocationtNameMaxSize, it will be truncated
     * @note if mapID is changed, SelectedLocations, CurrentLocation, and Progress are cleared.
     * @note MATTER change notifications are made for the attributes that change
     */
    bool ModifySupportedLocation( uint32_t                                     aLocationId, 
                                  const DataModel::Nullable<uint8_t>         & aMapId, 
                                  const CharSpan                             & aLocationName,
                                  const DataModel::Nullable<int16_t>         & aFloorNumber,
                                  const DataModel::Nullable<AreaTypeTag>     & aAreaType,
                                  const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                                  const DataModel::Nullable<PositionTag>     & aPositionTag,
                                  const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  );

    /**
     * @brief Clear the Supported Locations list
     * @return true if supported locations was not already null
     * 
     * @note if Supported Locations is cleared, SelectedLocations, CurrentLocation, and Progress are also cleared
     * @note MATTER change notifications are made for the attributes that change
     */
    bool ClearSupportedLocations();


    //*************************************************************************
    // Supported Maps manipulators

    /**
     * @brief Is the map in the supported maps list?
     * @param[in] aMapId id of the map
     * @return true if the map identified by Id is in the supported maps list
     * 
     * @note - use GetSupportedMapById if map contents are needed.
     */ 
    bool IsSupportedMap(uint8_t aMapId);

    /**
     * @brief Add new map to the supported maps list.
     * @param[in] aMapId The ID of the new added map.
     * @param[in] aMapName The name of the new added map (cannot be an empty string).
     * @return true if the new map passed validation checks and was successfully added to the list.
     * 
     * @note MATTER change notifications are made for the attributes that change.
     */ 
    bool AddSupportedMap(uint8_t aMapId, const CharSpan & aMapName);

    /**
     * @brief rename an existing map in the supported maps list
     * @param[in] aMapId id of the map
     * @param[in] newMapName new name of the map (cannot be empty string)
     * 
     * @return true if the new name passed validation checks and was successfully modified
     * @note if the specified map is not a member of the supported maps list, returns false with no action taken.
     * @note MATTER change notifications are made for the attributes that change
     */ 
    bool RenameSupportedMap(uint8_t aMapId, const CharSpan & newMapName);
    
    /**
     * @brief Clear the Supported Maps list
     * @return true if Supported Maps list was not already null
     * 
     * @note if list is cleared, SupportedLocations, SelectedLocations, CurrentLocation, and Progress are also cleared
     * @note MATTER change notifications are made for the attributes that change
     */
    bool ClearSupportedMaps();


    //*************************************************************************
    // Selected Locations manipulators

    /**
     * @brief Add a selected location
     * @param[in] aSelectedLocation locationID to add
     * @bool true if successfully added
    */
    bool AddSelectedLocation(uint32_t & aSelectedLocation);
    
    /**
     * @brief Is the location in the selected locations list?
     * @param[in] aLocationId id of the location
     * @return true if the location identified by Id is in the selected locations list
     */ 
    bool IsSelectedLocation(uint32_t aLocationId);

    /**
     * @brief Clear the Selected Locations list
     * @return true if selected locations was not already null
     * 
     * @note MATTER change notification is made if the attribute changes
     */
    bool ClearSelectedLocations();


    //*************************************************************************
    // Current Location manipulators

    /**
     * @brief Get the Current Location 
     * @return The current location
     */
    DataModel::Nullable<uint32_t> GetCurrentLocation();

    /**
     * @brief Set the current location
     * @param[in] aCurrentLocation where the device is currently. Must be a supported location or NULL.
     * @return true if the current location is set
     * 
     * @note if current location is set to null, estimated end time will be set to null
     * @note MATTER change notification is made if the attribute changes
     */ 
    bool SetCurrentLocation(const DataModel::Nullable<uint32_t> & aCurrentLocation);


    //*************************************************************************
    // Estimated End Time manipulators

    /**
     * @brief Get the estimated end time
     * @return The estimated epoch time in seconds when operation at the location indicated by the CurrentLocation attribute will be completed, in seconds.
     */
    DataModel::Nullable<uint32_t> GetEstimatedEndTime();

    /**
     * @brief Set the estimated end time.
     * @param[in] aEstimatedEndTime The estimated epoch time in seconds when operation at the location indicated by the CurrentLocation attribute will be completed, in seconds.
     * @return true if attribute is set.
     * 
     * @note MATTER change notification is made if the change meets the requirements for EstimatedEndTime notification (depends on old and new values).
     */
    bool SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime);


    //*************************************************************************
    // Progress list manipulators

    /**
     * @brief Is the progress element in the progress list?
     * @param[in] aLocationId location id of the progress element
     * @return true if the progress element identified by Id is in the progress list
     */ 
    bool IsProgressElement(uint32_t aLocationId);

    /**
     * @brief Add a progress element containing inactive status to the progress list
     * @param[in] aLocationId location id of the progress element
     * @return true if the new progress element passed validation checks and was successfully added to the list
     * 
     * @note MATTER change notification is made if the attribute changes
    */
    bool AddPendingProgressElement(uint32_t aLocationId);

    /**
     * @brief Set the status of progress element identified by locationID
     * @param[in] aLocationId the locationID to use
     * @param[in] status location cluster operation status for this location
     * @return true if progress element is found and status is set
     *
     * @note TotalOperationalTime is set to null if resulting opStatus is not equal to Completed or Skipped
     * @note MATTER change notification is made if the attribute changes
    */
    bool SetProgressStatus(uint32_t aLocationId, OperationalStatusEnum opStatus);

    /**
     * @brief Set the total operational time for the progress element identified by locationID
     * @param[in] aLocationId the locationID to use
     * @param[in] aTotalOperationalTime total operational time for this location.
     * @return true if progress element is found and operational time is set
     * 
     * @note MATTER change notification is made if the attribute changes
    */
    bool SetProgressTotalOperationalTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime);

    /**
     * @brief Set the estimated time for the  progress element identified by locationID
     * @param[in] aLocationId the locationID to use
     * @param[in] aEstimatedTime total operational time for thus location
     * @return true if progress element is found and operational time is set
     * 
     * @note MATTER change notification is made if the attribute changes
    */
    bool SetProgressEstimatedTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aEstimatedTime);

    /**
     * @brief Clear the progress list
     * @return true if selected locations was not already null
     * 
     * @note MATTER change notification is made if the attribute changes
     */
    bool ClearProgress();


    //*************************************************************************
    // Feature Map attribute

    /**
     * @brief Check if a feature is supported
     * @param[in] feature the feature enum
     * @return true if the feature is supported
     * 
     * @note the Service Area features are set at startup and are read-only to both device and client
     */
    bool HasFeature(ServiceArea::Feature feature) const;
};


} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
