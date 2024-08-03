/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "service-area-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

class Instance;

// *****************************************************************************
// cluster constraints

constexpr size_t kMaxNumSupportedLocations = 255;
constexpr size_t kMaxNumSupportedMaps      = 255;
constexpr size_t kMaxNumSelectedLocations  = 255;
constexpr size_t kMaxNumProgressElements   = 255;

constexpr size_t kMaxSizeStatusText = 256;

/**
 * ServiceArea::Delegate Defines methods for implementing application-specific
 * logic for the Service Area Cluster.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Stop this class objects from being copied.
     */
    Delegate(const Delegate &)             = delete;
    Delegate & operator=(const Delegate &) = delete;

    /**
     * @brief This method will be called during the ServiceArea server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic.
     */
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; };

    //*************************************************************************
    // Command handling support

    /**
     * @brief Can the selected locations be set by the client in the current operating mode?
     * @param[out] statusText text describing why selected locations cannot be set (if return is false).
     * @return true if the current device state allows selected locations to be set by user.
     *
     * @note The statusText field SHOULD indicate why the request is not allowed, given the current mode
     *       of the device, which may involve other clusters.
     */
    virtual bool IsSetSelectedLocationsAllowed(MutableCharSpan statusText) = 0;

    /**
     * Given a set of locations to be set to the SelectedLocations attribute, this method should check that
     * the set of locations as a whole is valid and reachable by the device.
     * If the set of locations is invalid, the locationStatus should be set to InvalidSet and
     * the statusText SHALL include a vendor-defined error description.
     *
     * The caller of this method will ensure that there are no duplicates is the list
     * and that all the locations in the set are valid supported locations.
     *
     * @param[in] req List of new selected locations.
     * @param[out] locationStatus Success if all checks pass, error code if failure.
     * @param[out] statusText text describing failure (see description above), size kMaxSizeStatusText.
     * @return true if success.
     *
     * @note If the SelectLocations command is allowed when the device is operating and the selected locations change to none, the
     * device must stop.
     */
    virtual bool IsValidSelectLocationsSet(const Commands::SelectLocations::DecodableType & req,
                                           SelectLocationsStatus & locationStatus, MutableCharSpan statusText) = 0;

    /**
     * @brief The server instance ensures that the SelectedLocations and CurrentLocation attributes are not null before
     * calling this method.
     * @param[out] skipStatusText text describing why current location cannot be skipped.
     * @return true if command is successful, false if the received skip request cannot be handled due to the current mode of the
     * device.
     *
     * @note skipStatusText must be filled out by the function on failure.
     *
     * @note If the device successfully accepts the request and the ListOrder feature is set to 1:
     *       The server SHALL stop operating at the current location.
     *       The server SHALL attempt to operate at the remaining locations on the SelectedLocations attribute list, starting with
     * the next entry. If the end of the SelectedLocations attribute list is reached, the server SHALL stop operating.
     *
     * @note If the device successfully accepts the request and the ListOrder feature is set to 0:
     *       The server SHALL stop operating at the current location.
     *       The server SHALL attempt to operate at the locations on the SelectedLocations attribute list where operating has not
     * been completed, using a vendor defined order. If the server has completed operating at all locations on the SelectedLocations
     * attribute list, the server SHALL stop operating.
     *
     * @note If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
     *       If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined
     * error description which can be used to explain the error to the user. For example, if the Status field is set to
     * InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed, given the current mode of the device,
     * which may involve other clusters.
     */
    virtual bool HandleSkipCurrentLocation(MutableCharSpan skipStatusText)
    {
        // device support of this command is optional
        CopyCharSpanToMutableCharSpan("Skip Current Location command not supported by device"_span, skipStatusText);
        return false;
    }

    //*************************************************************************
    // Supported Locations accessors

    /**
     * @return true if the current device state allows the SupportedLocations attribute to be updated.
     *
     * @note The SupportedLocations attribute list changes (adding or deleting entries,
     *       changing their MapID fields, changing the LocationID fields, or nulling the entire list)
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients,
     *       and the potential confusion for the users.
     *
     * @note The device implementation MAY allow supported location changes while operating if the device
     *       repopulates the SupportedMaps, SupportedLocations, CurrentLocation, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
     */
    virtual bool IsSupportedLocationsChangeAllowed() = 0;

    virtual uint32_t GetNumberOfSupportedLocations() = 0;

    /**
     * @brief Get a supported location using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aSupportedLocation  copy of the location contents, if found.
     * @return true if location found, false otherwise.
     */
    virtual bool GetSupportedLocationByIndex(uint32_t listIndex, LocationStructureWrapper & aSupportedLocation) = 0;

    /**
     * @brief Get a supported location that matches a locationID.
     * @param[in] aLocationId the locationID to search for.
     * @param[out] listIndex the location's index in the list, if found.
     * @param[out] aSupportedLocation  copy of the location contents, if found.
     * @return true if location found, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex,
                                          LocationStructureWrapper & aSupportedLocation);

    /**
     * This method is called by the server instance to add a new location to the list.
     * The server instance will ensure that the newLocation is a valid, unique location.
     * @param [in] newLocation new location to add.
     * @param [out] listIndex filled with the list index for the new location, if successful.
     * @return true if successful, false otherwise.

     * @note this function SHOULD double check that the added location won't exceed the maximum list size.
     */
    virtual bool AddSupportedLocation(const LocationStructureWrapper & newLocation, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing location in the list.
     * The server instance will ensure that the modifiedLocation is a valid, unique location.
     * @param[in] listIndex The index of the location being modified.
     * @param[in] modifiedLocation A location with the modified contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that newLocation's locationID matches the object at listIndex.
     */
    virtual bool ModifySupportedLocation(uint32_t listIndex, const LocationStructureWrapper & modifiedLocation) = 0;

    /**
     * @return true if supported locations was not already null, false otherwise.
     */
    virtual bool ClearSupportedLocations() = 0;

    /**
     * @brief Ensure that when the Supported locations is modified, the required restrictions for the SelectedLocations,
     * CurrentLocation, and Progress attributes are maintained.
     *
     * This method will be called by the SDK whenever the adherence to the restrictions for these attributes cannot be guaranteed.
     * For example, if there are deletions in the SupportedMops or SupportedLocations attributes, or if there are changes to their
     * IDs. This method will no be called if the changes made to the SupportedMops or SupportedLocations attributes, ensure that the
     * restrictions are adhered. For example, if there are additions or the modifications do not involve changing IDs in the
     * SupportedMops or SupportedLocations attributes.
     *
     * The default implementation will set the SelectedLocations, CurrentLocation, and Progress attributes to null.
     *
     * The user is free the redefine this method as their device may have more information on what has changed and may be able to
     * maintain the restrictions on these attributes by selectively editing them.
     */
    virtual void HandleSupportedLocationsUpdated();

    //*************************************************************************
    // Supported Maps accessors

    /**
     * @return true if the current device state allows the SupportedMaps attribute to be updated.
     *
     * @note The SupportedMaps attribute list changes (adding or deleting entries,
     *       changing their MapID fields, or nulling the entire list)
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients,
     *       and the potential confusion for the users.
     *
     * @note The device implementation MAY allow supported maps changes while operating if the device
     *       repopulates the SupportedLocations, CurrentLocation, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
     */
    virtual bool IsSupportedMapChangeAllowed() = 0;

    virtual uint32_t GetNumberOfSupportedMaps() = 0;

    /**
     * @brief Get a supported map using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aSupportedMap  copy of the map contents, if found.
     * @return true if a supported map is found, false otherwise.
     */
    virtual bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap) = 0;

    /**
     * @brief Get a supported map that matches a mapID.
     * @param[in] aMapId the mapID to search for.
     * @param[out] listIndex the map's index in the list, if found.
     * @param[out] aSupportedMap copy of the location contents, if found.
     * @return true if a supported map is found, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap);

    /**
     * This method is called by the server instance to add a new map to the list.
     * The server instance will ensure that the newMap is a valid, unique map.
     * @param[in] newMap The new map to add.
     * @param[out] listIndex filled with the list index of the new map, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added map won't exceed the maximum list size
     */
    virtual bool AddSupportedMap(const MapStructureWrapper & newMap, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing map in the list.
     * The server instance will ensure that the modifiedMap is a valid, unique map.
     * @param[in] listIndexThe index of the map being modified.
     * @param[in] modifiedMapA map with the modified contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that modifiedMap's mapID matches the object at listIndex.
     */
    virtual bool ModifySupportedMap(uint32_t listIndex, const MapStructureWrapper & modifiedMap) = 0;

    /**
     * @return true if supported maps was not already null, false otherwise.
     */
    virtual bool ClearSupportedMaps() = 0;

    //*************************************************************************
    // Selected Locations accessors

    virtual uint32_t GetNumberOfSelectedLocations() = 0;

    /**
     * @brief Get a selected location using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] selectedLocation the selected location value, if found.
     * @return true if a selected location is found, false otherwise.
     */
    virtual bool GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & selectedLocation) = 0;

    /**
     * @return true if the aLocationId locationID is found in the SelectedLocations list, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool IsSelectedLocation(uint32_t aLocationId);

    /**
     * This method is called by the server instance to add a new selected location to the list.
     * The server instance will ensure that the aLocationId references a SUPPORTED location, and is unique within selected
     * locations.
     * @param[in] aLocationId The new locationID to add.
     * @param[out] listIndex filled with the list index of the new location, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added location won't exceed the maximum list size.
     */
    virtual bool AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex) = 0;

    /**
     * @return true if selected locations was not already null, false otherwise.
     */
    virtual bool ClearSelectedLocations() = 0;

    //*************************************************************************
    // Progress accessors

    virtual uint32_t GetNumberOfProgressElements() = 0;

    /**
     * @brief Get a progress element using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     */
    virtual bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement) = 0;

    /**
     * @brief Get a progress element that matches a locationID.
     * @param[in] aLocationId the locationID to search for.
     * @param[out] listIndex the location's index in the list, if found.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex,
                                        Structs::ProgressStruct::Type & aProgressElement);

    /**
     * @brief Is the progress element in the progress list?
     * @param[in] aLocationId location id of the progress element.
     * @return true if the progress element identified by Id is in the progress list.
     */
    virtual bool IsProgressElement(uint32_t aLocationId);

    /**
     * This method is called by the server instance to add a new progress element to the list.
     * The server instance will ensure that the newProgressElement is a valid, unique progress element.
     * @param[in] newProgressElement The new element to add.
     * @param[out] listIndex is filled with the list index for the new element, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added element won't exceed the maximum list size.
     */
    virtual bool AddProgressElement(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing progress element in the list.
     * The server instance will ensure that the modifiedProgressElement is a valid and unique progress element.
     * @param[in] listIndex The list index of the progress element being modified.
     * @param[in] modifiedProgressElement modified element's contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that modifiedProgressElement's locationID matches the object at listIndex
     */
    virtual bool ModifyProgressElement(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) = 0;

    /**
     * @return true if progress list was not already null, false otherwise.
     */
    virtual bool ClearProgress() = 0;

    Instance * GetInstance() { return mInstance; }

    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

private:
    Instance * mInstance = nullptr;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
