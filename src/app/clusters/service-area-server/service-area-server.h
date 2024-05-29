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
#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>

#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

#include <list>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {


// *****************************************************************************
// cluster constraints

constexpr size_t   kMaxNumSupportedLocations  = 255;
constexpr size_t   kMaxNumSupportedMaps       = 255;
constexpr size_t   kMaxNumSelectedLocations   = 255;
constexpr size_t   kMaxNumSupportedProgress   = 255;


/**
 * Uncopyable is a helper to ensure that Service Area cluster instances and delegates
 * are unique and cannot be copied.
 */
class Uncopyable
{
protected:
    Uncopyable() {}
    ~Uncopyable() = default;

private:
    Uncopyable(const Uncopyable &)             = delete;
    Uncopyable & operator=(const Uncopyable &) = delete;
};


class Delegate;


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
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeature The supported features of this Service Area Cluster
     * @param aClusterId The Id of the Service Area cluster to be instantiated.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, BitMask<ServiceArea::Feature> aFeature, ClusterId aClusterId = Clusters::ServiceArea::Id);


    ~Instance() override;

    /**
     * @brief Initialise the Service Area server instance.
     * This function must be called after defining an Instance class object.
     * @return an error if the given endpoint and cluster Id have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid return value will be CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();


private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;

    // Attribute Data Store
    std::map<uint32_t, LocationStructureWrapper>       mSupportedLocations;  // class includes text storage for name field & utility functions
    std::map<uint8_t, MapStructureWrapper>             mSupportedMaps;       // class includes text storage for name field & utility functions
    std::list<uint32_t>                                mSelectedLocations;   // If this attribute lists more than one location, and the ListOrder feature is set to 1, 
                                                                             // the device SHALL attempt to operate at the selected locations in the order they're listed in.
    DataModel::Nullable<uint32_t>                      mCurrentLocation;     // may be volatile - callback to device to refresh value is provided
    DataModel::Nullable<uint32_t>                      mEstimatedEndTime;    // may be volatile - callback to device to refresh value is provided
    std::map<uint32_t, Structs::ProgressStruct::Type>  mProgressList;        // may be volatile - callback to device to refresh value is provided
    BitMask<ServiceArea::Feature>                      mFeature;


    // aliases for container types
    using SupportedLocationPairType = decltype(mSupportedLocations)::value_type;
    using SupportedLocationType     = decltype(mSupportedLocations)::mapped_type;
    using SupportedMapPairType      = decltype(mSupportedMaps)::value_type; 
    using SupportedMapType          = decltype(mSupportedMaps)::mapped_type;
    using ProgressPairType          = decltype(mProgressList)::value_type; 
    using ProgressType              = decltype(mProgressList)::mapped_type;


    //*************************************************************************
    // core functions

    /**
     * @brief Inherited from AttributeAccessInterface
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;


    //*************************************************************************
    // attribute readers

    /**
     * @brief Read the Supported Locations
     * @param aEncoder      attribute value encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSupportedLocations(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * @brief Read the Supported Maps Attribute
     * @param aEncoder      attribute value encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSupportedMaps(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * @brief Read the Selected Locations Attribute
     * @param aEncoder attribute value encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadSelectedLocations(chip::app::AttributeValueEncoder & aEncoder);

    /**
     * Read the Progress Attribute
     * @param aEncoder attribute value encoder.
     * @return appropriately mapped CHIP_ERROR if applicable
    */
    CHIP_ERROR ReadProgress(chip::app::AttributeValueEncoder & aEncoder);


    //*************************************************************************
    // command handlers

    /**
     * @brief Handle Command: SelectLocations.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * @param req the command information
     * If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * 
     */
    void HandleSelectLocationsCmd(HandlerContext & ctx, const Commands::SelectLocations::DecodableType & req);

    /**
     * @brief check a list of locations against Location cluster requirements for selected locations
     * @param selectLocations list of locations to validate
     * @param locationStatus Success if all checks pass, SelectLocations command error code if fallure.
     * @param locationStatusText contains error description if appropriate
     * @return true if all checks pass, 
     */
    bool AreSetSelectLocationsParamsValid(const std::vector<uint32_t> & selectLocations,
                                          SelectLocationsStatus & locationStatus,
                                          std::string & locationStatusText);
                            
    /**
     * @brief Handle Command: SelectLocations.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * 
     */
    void HandleSkipCurrentCmd(HandlerContext & ctx);


public:

    //*************************************************************************
    // attribute notifications

    /**
     * @brief Notify MATTER that supported locations have changed
     */
    void NotifySupportedLocationsChanged();

    /**
     * @ brief Notify MATTER that supported maps have changed
     */
    void NotifySupportedMapsChanged();

    /**
     * @ brief Notify MATTER that selected locations have changed
     */
    void NotifySelectedLocationsChanged();

    /**
     * @ brief Notify MATTER that current location has changed
     */
    void NotifyCurrentLocationChanged();

    /**
     * @ brief Notify MATTER that current location has changed
     */
    void NotifyEstimatedEndTimeChanged();

    /**
     * @ brief Notify MATTER that progress list has changed
     */
    void NotifyProgressChanged();


    //*************************************************************************
    // Supported Locations manipulators

    /**
     * @brief Get the list of currently supported location Id's
     * @param aSupportedLocationIds empty vector to be filled with location Id's
    */
    void GetSupportedLocationIds(std::vector<uint32_t> & aSupportedLocationIds);

    /**
     * @brief Get information about a supported location using the locationId
     * @param aLocationId the locationId to use
     * @param aSupportedLocation  const pointer to the supported location object - null if not found
     * @return bool if location found
     * @note location pointer has limited lifetime - only valid until supported locations list is changed
    */
    bool GetSupportedLocationById(uint32_t aLocationId, const LocationStructureWrapper*& aSupportedLocation);

    /**
     * @brief Is the location in the supported locations list?
     * @param aLocationId id of the location
     * @return true if the location identified by Id is in the supported locations list
     */ 
    bool IsSupportedLocation(uint32_t aLocationId);

    /**
     * @brief check Location against cluster requirements for supported locations
     * @param aLocation id of the location
     * @return true if meets all checks
     */
    bool IsValidSupportedLocation(const LocationStructureWrapper & aLocation);

    /**
     * @brief check if location contents are unique with regard to supported locations
     * @param aLocation id of the location
     * @return true if meets all checks
     * @note this can be used to check a modified location that is already a member of supported locations (does not check against supported location with same locationId)
     * @note uniqueness of LocationId is checked whenever locations are added to the supported locations list.
     */
    bool IsUniqueSupportedLocation(const LocationStructureWrapper & aLocation);

    /**
     * @brief Add new location to the supported locations list
     * @param aLocationId unique identifier of this location
     * @param aMapId identifier of supported map
     * @param aLocationName human readable name for this location (empty string if not used)
     * @param aFloorNumber represents floor level - negative values for below ground
     * @param aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param aLandmarkTag common namespace Landmak tag - indicates an association of the location with a home landmark
     * @param aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * @return true if the new location passed validation checks and was successfully added to the list
     * @note caller is responsible for change notification (caller may be making more than one call to AddSupportedLocation)
     * @note if aLocationName is larger than kLocationtNameMaxSize, it will be truncated

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
     * @brief  Modify/replace an existing location in the supported locations list
     * @param aLocationId unique identifier of this location
     * @param aMapId identifier of supported map
     * @param aLocationName human readable name for this location (empty string if not used)
     * @param aFloorNumber represents floor level - negative values for below ground
     * @param aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param aLandmarkTag common namespace Landmak tag - indicates an association of the location with a home landmark
     * @param aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * @return true if the location is a member of supported locations, the modifications pass all validation checks and the location was modified 
     * @note caller is responsible for change notification
     * @note if aLocationName is larger than kLocationtNameMaxSize, it will be truncated

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
     * @brief When a supported location changes or is deleted, update other attributes as required
     * @param aLocationId unique identifier of this location
     * 
     * @note Any selected locations that depend on the modified supported location will be deleted.
     * @note If the device is operating and selected locations becomes null, the device shall stop operation.
     * @note Any progress elements that depend in the modified supported location will be deleted.
     * @note if the updated location is the current location, the current location will be set to null.
     * @note MATTER change notifications are made for any attributes that change
     */
    void HandleSupportedLocationModified(uint32_t aLocationId);

    /**
     * @brief remove any supported locations with invalid map references (mapId not in supported maps)
     * @return true if any locations were removed from supported locations.
     * 
     * @note Any selected locations that depend on the deleted supported locations will be deleted.
     * @note If the device is operating and selected locations becomes null, the device shall stop operation.
     * @note Any progress elements that depend in the deleted supported locations will be deleted.
     * @note if the updated location is the current location, the current location will be set to null.
     * @note MATTER change notifications are made for any attributes that change
     */
    bool PruneSupportedLocations();

    /**
     * @brief remove location from supported location list
     * @param aLocationId unique identifier of this location
     * @return true if the location was found in supported locations and deleted.
     * @note if the specified location is not a member of the supported location list, returns false with no action taken.
     * @note caller is responsible for change notification
     */
    bool DeleteSupportedLocation(uint32_t aLocationId);

    /**
     * @brief Clear the Supported Locations list
     * @return true if supported locations was not already null
     * @note SelectedLocations, CurrentLocation, and Progress are set to null.
     * @note MATTER change notifications are made for the attributes that change
     */
    bool ClearSupportedLocationsList();


    //*************************************************************************
    // Supported Maps manipulators

    /**
     * @brief Get a list of currently supported map Id's
     * @param aMapLocationIds empty vector to be filled with location Id's
    */
    void GetSupportedMapIds(std::vector<uint8_t> & aSupportedMapIds);

    /**
     * @brief Get a supported map name using the mapId
     * @param aMapId the mapId to use
     * @param aMapName const pointer to the found map name - NULL if not found
     * @return true if map found
    */
    bool GetSupportedMapNameById(uint8_t aMapId, CharSpan & aMapName);

    /**
     * @brief Is the map in the supported maps list?
     * @param aMapId id of the map
     * @return true if the map identified by Id is in the supported maps list
     */ 
    bool IsSupportedMap(uint8_t aMapId);

    /**
     * @brief Add new map to the supported maps list
     * @param aMapId id of the new added map
     * @param aMapName name of the new added map
     * @return true if the new map passed validation checks and was successfully added to the list
     */ 
    bool AddSupportedMap(uint8_t aMapId, const CharSpan & aMapName);

    /**
     * @brief rename an existing map in the supported maps list
     * @param aMapId id of the map
     * @param aMapName new name of the map (cannot be empty string)
     * @return true if the new name passed validation checks and was successfully modified
     * @note if the specified map is not a member of the supported maps list, returns false with no action taken.
     * @note caller is responsible for change notification
     * 
     */ 
    bool RenameSupportedMap(uint8_t aMapId, const CharSpan & aMapName);

    /**
     * @brief remove map from supported maps list
     * @param aMapId unique identifier of this map
     * @return true if the map was found in supported maps and deleted.
     * @note if the specified map is not a member of the supported map list, returns false with no action taken.
     * 
     * @note Any supported locations that depend on the deleted mapId will be deleted.
     * @note Any selected locations that depend on the deleted supported locations will be deleted.
     * @note If the device is operating and selected locations becomes null, the device shall stop operation.
     * @note Any progress elements that depend in the deleted supported locations will be deleted.
     * @note if the updated location is the current location, the current location will be set to null.
     * @note MATTER change notifications are made for any attributes that change
     */
    bool DeleteSupportedMap( uint8_t aMapId);
    
    /**
     * @brief Clear the Supported Maps list
     * @return true if Supported Maps list was not already null
     * 
     * @note Any supported locations that depend on mapId's will be deleted.
     * @note Any selected locations that depend on the deleted supported locations will be deleted.
     * @note If the device is operating and selected locations becomes null, the device shall stop operation.
     * @note Any progress elements that depend in the deleted supported locations will be deleted.
     * @note if the updated location is the current location, the current location will be set to null.
     * @note MATTER change notifications are made for any attributes that change
     */
    bool ClearSupportedMapsList();

    //*************************************************************************
    // Selected Locations manipulators

    /**
     * @brief Add a selected location
     * @param aSelectedLocation locationId to add
     * @note Selected locations are normally only set through the SelectLocations command.
     * @note This function is intended primarily for testing.
    */
    bool AddSelectedLocation(uint32_t & aSelectedLocation);

    /**
     * @brief Get the list of selected locations
     * @param aSelectedLocations empty vector to be filled with location Id's
    */
    void GetSelectedLocations(std::vector<uint32_t> & aSelectedLocations);
    
    /**
     * @brief Is the location in the selected locations list?
     * @param aLocationId id of the location
     * @return true if the location identified by Id is in the selected locations list
     */ 
    bool IsSelectedLocation(uint32_t aLocationId);

    /**
     * @brief remove any selected locations which are no longer supported location (locationId not in supported locations)
     * @return true if any locations were removed from selected locations.
     * @note caller is responsible for change notification
     */
    bool PruneSelectedLocations();

    /**
     * @brief remove a location from the selected locations list
     * @return true if the location was removed from selected locations.
     * @note caller is responsible for change notification
     */
    bool DeleteSelectedLocation(uint32_t aLocationId);

    /**
     * @brief Clear the Selected Locations list
     * @note caller is responsible for change notification
     * @return the Selected Locations list changed (i.e. it previously had entries)
     */
    bool ClearSelectedLocationsList();


    //*************************************************************************
    // Current Location manipulators

    /**
     * @brief Get the Current Location 
     * @return The current location
     */
    DataModel::Nullable<uint32_t> GetCurrentLocation();

    /**
     * @brief Set the current location
     * @param aCurrentLocation where the device is currently. Must be a supported location or NULL.
     * @return true if the current location is set
     * @note if current location is set to null, estimated end time will be set to null
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
     * @brief Set the estimated end time
     * @param aEstimatedEndTime The estimated epoch time in seconds when operation at the location indicated by the CurrentLocation attribute will be completed, in seconds.
     * @return true if attribute is set
     * @note change notification is generated (!!) if appropriate per cluster requirements
     */ 
    bool SetEstimatedEndTime(const DataModel::Nullable<uint32_t> & aEstimatedEndTime);


    //*************************************************************************
    // Progress list manipulators

    /**
     * @brief Get the list of current progress element Id's
     * @param aSupportedLocationIds empty vector to be filled with location Id's
    */
    void GetProgressElementIds(std::vector<uint32_t> & aSupportedLocationIds);

    /**
     * @brief Get a progress element using the locationId
     * @param aLocationId the locationId to use
     * @param aProgressElement const pointer to the found map name - NULL if not found
     * @return true if element is found and returned
     * @note progress element pointer has limited lifetime - only valid until progress list is changed
     * @note change progress element contents with provided set functions
    */
    bool GetProgressElementById(uint32_t aLocationId, const Structs::ProgressStruct::Type*& aProgressElement);

    /**
     * @brief Is the progress element in the progress list?
     * @param aLocationId location id of the progress element
     * @return true if the progress element identified by Id is in the progress list
     */ 
    bool IsProgressElement(uint32_t aLocationId);

    /**
     * @brief Add a progress element containing inactive status to the progress list
     * @param aLocationId location id of the progress element
     * @return true if the new progress element passed validation checks and was successfully added to the list
     * @note caller is responsible for change notification
    */
    bool AddPendingProgressElement(uint32_t aLocationId);

    /**
     * @brief Set the status of progress element identified by locationId
     * @param aLocationId the locationId to use
     * @param status location cluster operation status for this location
     * @return true if progress element is found and status is set
     * @note caller is responsible for change notification
     * @note TotalOperationalTime is set to null if opStatus is not Completed or Skipped
    */
    bool SetProgressStatus(uint32_t aLocationId, OperationalStatusEnum opStatus);

    /**
     * @brief Set the total operational time for the progress element identified by locationId
     * @param aLocationId the locationId to use
     * @param aTotalOperationalTime total operational time for thus location
     * @return true if progress element is found and operational time is set
     * @note caller is responsible for change notification
    */
    bool SetProgressTotalOperationalTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aTotalOperationalTime);

    /**
     * @brief Set the estimated time for the  progress element identified by locationId
     * @param aLocationId the locationId to use
     * @param aEstimatedTime total operational time for thus location
     * @return true if progress element is found and operational time is set
     * @note caller is responsible for change notification
    */
    bool SetProgressEstimatedTime(uint32_t aLocationId, const DataModel::Nullable<uint32_t> & aEstimatedTime);

    /**
     * @brief remove any progress elements which no longer reference a supported location (locationId not in supported locations)
     * @return true if any progress elements were removed.
     * @note caller is responsible for change notification
     */
    bool PruneProgressElements();

        /**
     * @brief remove a location from the progress list
     * @return true if the location was removed from the progress list.
     * @note caller is responsible for change notification
     */
    bool DeleteProgressElement(uint32_t aLocationId);

    /**
     * @brief Clear the progress list
     * @note caller is responsible for change notification
     * @return the Progress list changed (i.e. it previously had entries)

     */
    bool ClearProgressList();


    //*************************************************************************
    // Feature Map attribute

    /**
     * @brief Check if a feature is supported
     * @param feature the feature enum
     * @return true if the feature is supported
     * @note the Service Area features are set at startup and are read-only to both device and client
     */

    bool HasFeature(ServiceArea::Feature feature) const;
};


/**
 * Location::Delegate Defines methods for implementing application-specific
 * logic for the Location Cluster.
 */
class Delegate : public Uncopyable
{
public:
    Delegate() = default;
    virtual ~Delegate() = default;

private:

    friend class Instance;
    Instance * mInstance = nullptr;

    /**
     * @brief This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }


    //*************************************************************************
    // volatile attribute support

    /**
     * @brief give the device a chance to update current location
     * @param prevCurrentLocation "current" current location value - used to determine if update notification is triggered
     * @note this is called by the instance function GetCurrentLocation() to ensure the latest value is available
     * @note default implementation can be used if current location is non-volatile (always expected to be up-to-date)
    */
    virtual void HandleVolatileCurrentLocation(const DataModel::Nullable<uint32_t> prevCurrentLocation) = 0;

    /**
     * @brief give the device a chance to update estimated end time
     * @param prevEstimatedEndTime "current" estimated end time value - used to determine if update notification is triggered
     * @note this is called by the instance function GetEstimatedEndTime() to ensure the latest value is available
     * @note default implementation can be used if progress entries are non-volatile (always expected to be up-to-date)
    */
    virtual void HandleVolatileEstimatedEndTime(const DataModel::Nullable<uint32_t> prevEstimatedEndTime) = 0;

    /**
     * @brief give the device a chance to update progress elements
     * @note this is called by the instance functions ReadProgress() and  GetEstimatedEndTime() to ensure the latest values are available
     * @note default implementation can be used if progress elements are non-volatile (always expected to be up-to-date)
     * @note The EstimatedTime field is the most likely to be a volatile value
    */
    virtual void HandleVolatileProgressList() = 0;


    //*************************************************************************
    // Selected Locations "pruning" support

    /**
     * @brief the device has changed supported maps or supported locations, leading to a change in selected locations.
     * @note If the device does not make or allow changes to supported maps or supported locations while operating, it's likely no action is needed
     * @note If selected locations is changed such that no locations are selected and the device is operating, the device must stop.
     * @note If selected locations is changed and the device is operating, the device may need to update it's operating sequence
     * @note If the device is operating and does not allow changing selected locations while operating, the device still needs to handle this change somehow (e.g. abort operation).
     *  (changes to maps or supported locations may result in changes in selected locations, so change may occur as a side-effect of updates)
     */
    virtual void HandleSelectedLocationsChanged() = 0;


    //*************************************************************************
    // Command handling support

    /**
     * @brief can the selected locations be set by the client in the current operating mode?
     * @param statusText text describing why selected locations cannot be set
     * @return true if the current device state allows selected locations to be set by user
     * @note locationStatusText to be filled with short text giving details of failure if return value is false
    */
    virtual bool IsSetSelectedLocationAllowed(std::string & statusText) = 0;

    /**
     * @brief New selected locations have been requested by the client.
     * @param newSelectLocations List of new selected locations
     * @param locationStatus  Success if all checks pass, error code if fallure.
     * @param locationStatusText text describing failure
     * @return true if success
     * @note newSelectLocations MUST be pre-checked for no duplicates and no unsupported locations before this function is called
     * @note IsSetSelectedLocationAllowed() MUST be called with a return value of true before this function can be called.
     * @note Fail with InvalidSet if newSelectLocations contains values the device cannot handle (e.g. locations on different floors), with failure description in locationStatusText
     * @note If select locations is allowed when the device is operating and selected locations change such that none are selected, the device must stop.
     * @note The Selected Locations attribute should be updated only after this function returns true.
     */
    virtual bool HandleSetSelectLocations(const std::vector<uint32_t> & newSelectLocations, SelectLocationsStatus & locationStatus, std::string & locationStatusText) = 0;

    /**
     * @brief ask the device attempt to skip the current location
     * @param skipStatusText text describing why current location cannot be skipped
     * @return true if command is successful, failure if the received skip request cannot be handled due to the current mode of the device.
     * @note instance code checks current location and selected locations before calling this function
     * @note skipStatusText must be filled out by the function on failure.
     * 
     * If the device successfully accepts the request and the ListOrder feature is set to 1:
     * 
     * The server SHALL stop operating at the current location.
     * The server SHALL attempt to operate at the remaining locations on the SelectedLocations attribute list, starting with the next entry.
     * If the end of the SelectedLocations attribute list is reached, the server SHALL stop operating.
     * 
     * If the device successfully accepts the request and the ListOrder feature is set to 0:
     * 
     * The server SHALL stop operating at the current location.
     * The server SHALL attempt to operate at the locations on the SelectedLocations attribute list where operating has not been completed, using a vendor defined order.
     * If the server has completed operating at all locations on the SelectedLocations attribute list, the server SHALL stop operating.
    */
    virtual bool HandleSkipCurrentLocation(std::string & skipStatusText)
    {
        skipStatusText = "Skip Current Location command not supported by device";
        return false;
    }


protected:
    Instance * GetInstance() const { return mInstance; }
};


} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
