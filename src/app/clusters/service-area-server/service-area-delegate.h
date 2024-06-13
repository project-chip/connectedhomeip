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

// *****************************************************************************
// cluster constraints

constexpr size_t   kMaxNumSupportedLocations  = 255;
constexpr size_t   kMaxNumSupportedMaps       = 255;
constexpr size_t   kMaxNumSelectedLocations   = 255;
constexpr size_t   kMaxNumProgressElements    = 255;

constexpr size_t   kMaxSizeStatusText = 256;


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


/**
 * ServiceArea::Delegate Defines methods for implementing application-specific
 * logic for the Service Area Cluster.
 */
class Delegate : public Uncopyable
{
public:
    Delegate() = default;
    virtual ~Delegate() = default;

protected:

    friend class Instance;

    //*************************************************************************
    // Command handling support

    /**
     * @brief can the selected locations be set by the client in the current operating mode?
     * @param[out] statusText text describing why selected locations cannot be set (if return is false)
     * @return true if the current device state allows selected locations to be set by user
     * 
     * @note The statusText field SHOULD indicate why the request is not allowed, given the current mode
     *       of the device, which may involve other clusters.
    */
    virtual bool IsSetSelectedLocationAllowed(char* statusText) = 0;

    /**
     * @brief New selected locations have been requested by the client.
     * @param[in] newSelectLocations List of new selected locations
     * @param[out] locationStatus Success if all checks pass, error code if fallure.
     * @param[out] statusText text describing failure (see notes), size kMaxSizeStatusText + 1 byte for terminating character
     * @param[out] useStatusText if true, the statusText value should be returned in the command response
     * @return true if success
     * 
     * @note newSelectLocations pre-checked no unsupported locations before this function is called.
     * @note IsSetSelectedLocationAllowed() MUST be called with a return value of true before this function can be called. (InvalidInMode)
     * @note Fail with DuplicatedLocations if the same value is selected more than once.
     *       Fail with InvalidSet if newSelectLocations contains values the device cannot handle (e.g. locations on different floors), with failure description in locationStatusText
     * 
     * @note If select locations is allowed when the device is operating and selected locations change such that none are selected, the device must stop.
     * @note The Selected Locations attribute should be updated only after this function returns true.
     * 
     * @note If the Status field is set to Success, the StatusText field is optional.
     *       If the Status field is set to UnsupportedLocation, the StatusText field SHALL be an empty string.
     *       If the Status field is set to DuplicatedLocations, the StatusText field SHALL be an empty string.
     *       If the Status field is not set to Success, or UnsupportedLocation, or DuplicatedLocations, the StatusText field SHALL include a vendor-defined error description
     *       which can be used to explain the error to the user. For example,
     *       if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate
     *       why the request is not allowed, given the current mode
     *       of the device, which may involve other clusters.
    */
    virtual bool HandleSetSelectLocations(const Commands::SelectLocations::DecodableType & req, 
                        SelectLocationsStatus & locationStatus, char* statusText, bool & useStatusText) = 0;

    /**
     * @brief ask the device to attempt to skip the current location
     * @param[out] skipStatusText text describing why current location cannot be skipped
     * @return true if command is successful, failure if the received skip request cannot be handled due to the current mode of the device.
     * 
     * @note server instance code checks current location and selected locations before calling this function
     * @note skipStatusText must be filled out by the function on failure.
     * 
     * @note If the device successfully accepts the request and the ListOrder feature is set to 1:       
     *       The server SHALL stop operating at the current location.
     *       The server SHALL attempt to operate at the remaining locations on the SelectedLocations attribute list, starting with the next entry.
     *       If the end of the SelectedLocations attribute list is reached, the server SHALL stop operating.
     * 
     * @note If the device successfully accepts the request and the ListOrder feature is set to 0:
     *       The server SHALL stop operating at the current location.
     *       The server SHALL attempt to operate at the locations on the SelectedLocations attribute list where operating has not been completed, using a vendor defined order.
     *       If the server has completed operating at all locations on the SelectedLocations attribute list, the server SHALL stop operating.
     * 
     * @note If the Status field is set to InvalidLocationList, the StatusText field SHALL be an empty string.
     *       If the Status field is not set to Success, or InvalidLocationList, the StatusText field SHALL include a vendor defined error description
     *       which can be used to explain the error to the user. For example, if the Status field is set to InvalidInMode, the StatusText field SHOULD indicate
     *       why the request is not allowed, given the current mode
     *       of the device, which may involve other clusters.
    */
    virtual bool HandleSkipCurrentLocation(char* skipStatusText)
    {
        // device support of this command is optional
        strncat(skipStatusText, "Skip Current Location command not supported by device", kMaxSizeStatusText);
        return false;
    }


    //*************************************************************************
    // Supported Locations accessors

    /**
     * @brief can the supported locations be set in the current operating mode?
     * @return true if the current device state allows supported location updates
     * 
     * @note The SupportedLocations attribute list changes (adding or deleting entries, 
     *       changing their MapID fields, changing the LocationID fields, or nulling the entire list) 
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients, 
     *       and the potential confusion for the users.
     * 
     * @note The device implementation MAY allow supported location changes while operating if the device
     *       repopulates the SupportedMaps, SupportedLocations, CurrentLocation, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
     * 
    */
    virtual bool IsSupportedLocationChangeAllowed() = 0;

    /**
     * @brief Get the number of entries in the supported locations attribute
     * @return the number of entries
     */
    virtual uint32_t GetNumberOfSupportedLocations() = 0;

    /**
     * @brief Get a supported location using the position in the list
     * @param[in] listIndex the position in the list
     * @param[out] aSupportedLocation  copy of the location contents - if found
     * @return true if location found
     * 
     * @note can be used to iterate through supported locations
    */
    virtual bool GetSupportedLocationByIndex(uint32_t listIndex, LocationStructureWrapper & supportedLocation) = 0;

    /**
     * @brief Get a supported location that matches a locationID
     * @param[in] aLocationId the locationID to search for
     * @param[out] listIndex the location's index in the list, if found
     * @param[out] aSupportedLocation  copy of the location contents, if found
     * @return true if location found
     * 
     * @note may be overloaded in device implementation for optimization, if desired
    */
    virtual bool GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex, LocationStructureWrapper & aSupportedLocation)
    {
        bool ret_value = false;

        listIndex = 0;

        // simple linear iteration to find the location with the desired locationID.
        while (GetSupportedLocationByIndex(listIndex, aSupportedLocation))
        {
            if (aSupportedLocation.locationID == aLocationId)
            {
                ret_value = true;
                break;
            }

            ++listIndex;
        }

        return ret_value;
    };

    /**
     * @brief Add a location to the supported locations list.
     * @param [in] newLocation new location to add
     * @param [out] listIndex filled with the list index for the new location, if successful
     * @return true if successful

     * @note caller MUST guarantee newLocation is a valid, unique location
     * @note this function SHOULD double check that the added location won't exceed the maximum list size
     */
    virtual bool AddSupportedLocation(const LocationStructureWrapper & newLocation, uint32_t & listIndex) = 0;

    /**
     * @brief Modify a location in the supported locations list.
     * @param[in] listIndex list index of the location being modified
     * @param[in] modifedLocation modified location contents
     * @return true if successful
     * 
     * @note caller MUST guarantee modified location is a valid, unique location
     * @note this function SHOULD double check that newLocation's locationID matches the object at listIndex
     */
    virtual bool ModifySupportedLocation(uint32_t listIndex, const LocationStructureWrapper & modifiedLocation) = 0;

    /**
     * @brief Clear the Supported Locations list
     * @return true if supported locations was not already null
     * 
     * @note no notifications or other side effects
     */
    virtual bool ClearSupportedLocations() = 0;


    //*************************************************************************
    // Supported Maps accessors

    /**
     * @brief can the supported maps attribute be changed in the current operating mode?
     * @return true if the current device state allows supported maps updates
     * 
     * @note The SupportedMapss attribute list changes (adding or deleting entries, 
     *       changing their MapID fields, or nulling the entire list) 
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients, 
     *       and the potential confusion for the users.
     * 
     * @note The device implementation MAY allow supported maps changes while operating if the device
     *       repopulates the SupportedLocations, CurrentLocation, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
    */
    virtual bool IsSupportedMapChangeAllowed() = 0;

    /**
     * @brief Get the number of entries in the supported maps attribute
     * @return the number of entries
     */
    virtual uint32_t GetNumberOfSupportedMaps() = 0;

    /**
     * @brief Get a supported map using the position in the list
     * @param[in] listIndex the position in the list
     * @param[out] aSupportedMap  copy of the map contents - if found
     * @return true if a supported map is found.
     * @note can be used to iterate through supported locations
    */
    virtual bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & supportedMap) = 0;

    /**
     * @brief Get a supported map that matches a mapID
     * @param[in] aMapId the mapID to search for
     * @param[out] listIndex the map's index in the list, if found
     * @param[out] aSupportedMap copy of the location contents, if found
     * @return true if a supported map is found.
     * 
     * @note may be overloaded in device implementation for optimization, if desired
    */
    virtual bool GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
    {
        bool ret_value = false;

        listIndex = 0;

        while (GetSupportedMapByIndex(listIndex, aSupportedMap))
        {
            if (aSupportedMap.mapID == aMapId)
            {
                ret_value = true;
                break;
            }

            ++listIndex;
        }

        return ret_value;
    };

    /**
     * @brief Add a map to the supported maps list.
     * @param[in] newMap new map to add
     * @param[out] listIndex filled with the list index of the new map, if successful
     * @return true if successful
     * 
     * @note caller MUST guarantee newMap is a valid, unique map
     * @note this function SHOULD double check that the added map won't exceed the maximum list size
     */
    virtual bool AddSupportedMap(const MapStructureWrapper & newMap, uint32_t & listIndex) = 0;

    /**
     * @brief Modify a map in the supported maps list.
     * @param modifedMap[in] modified map contents
     * @param listIndex[in] list index of the location being modified
     * @return true if successful
     * 
     * @note caller MUST guarantee modified map is a valid, unique map
     * @note this function SHOULD double check that modifedMap's mapID matches the object at listIndex
     */
    virtual bool ModifySupportedMap(uint32_t listIndex, const MapStructureWrapper & modifiedMap) = 0;

    /**
     * @brief Clear the Supported Maps list
     * @return true if supported locations was not already null
     * 
     * @note no notifications or other side effects
     */
    virtual bool ClearSupportedMaps() = 0;


    //*************************************************************************
    // Selected Locations accessors

    /**
     * @brief Get the number of entries in the selected locations attribute
     * @return the number of entries
     */
    virtual uint32_t GetNumberOfSelectedLocations() = 0;

    /**
     * @brief Get a selected location using the position in the list
     * @param[in] listIndex the position in the list
     * @param[in] selectedLocation the selected location value, if found
     * @return true if a selected location is found.
     * 
     * @note can be used to iterate through supported locations
    */
    virtual bool GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & selectedLocation) = 0;

    /**
     * @brief Check if a locationID is in the selected locations list
     * @param[in] aLocationId the locationID to search for
     * @return bool if location found
     * 
     * @note may be overloaded in device implementation for optimization, if desired
    */
    virtual bool IsSelectedLocation(uint32_t aLocationId)
    {
        bool ret_value = false;

        uint32_t listIndex = 0;
        uint32_t selectedLocation;

        while (GetSelectedLocationByIndex(listIndex, selectedLocation))
        {
            if (selectedLocation == aLocationId)
            {
                ret_value = true;
                break;
            }

            ++listIndex;
        }

        return ret_value;
    };

    /**
     * @brief Add a locationID to the selected locations list.
     * @param[in] aLocationId new locationID to add
     * @param[in] listIndex filled with the list index of the new location, if successful
     * @return true if successful
     * 
     * @note selected locations are normally set through the SelectLocations command, 
     *       this function is provided for testing
     * 
     * @note caller MUST guarantee aLocationId references a SUPPORTED location, and is unique within selected locations
     * @note this function SHOULD double check that the added location won't exceed the maximum list size
     */
    virtual bool AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex) = 0;

    /**
     * @brief Clear the Selected Locations list
     * @return true if selected locations was not already null
     * 
     * @note no notifications or other side effects
     */
    virtual bool ClearSelectedLocations() = 0;


    //*************************************************************************
    // Progress accessors

    /**
     * @brief Get the number of entries in the progress attribute
     * @return the number of entries
     */
    virtual uint32_t GetNumberOfProgressElements() = 0;

    /**
     * @brief Get a progress element using the position in the list
     * @param[in] listIndex the position in the list
     * @param[out] aProgressElement  copy of the progress element contents - if found
     * @return true if a progress element is found.
     * 
     * @note can be used to iterate through the progress list
    */
    virtual bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & progressElement) = 0;

    /**
     * @brief Get a progress element that matches a locationID
     * @param[in] aLocationId the locationID to search for
     * @param[out] listIndex the location's index in the list, if found
     * @param[out] aProgressElement  copy of the progress element contents, if found
     * @return true if a progress element is found.
     * 
     * @note may be overloaded in device implementation for optimization, if desired
    */
    virtual bool GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
    {
        bool ret_value = false;

        listIndex = 0;

        // simple linear iteration to find the progress element with the desired locationID.
        while (GetProgressElementByIndex(listIndex, aProgressElement))
        {
            if (aProgressElement.locationID == aLocationId)
            {
                ret_value = true;
                break;
            }

            ++listIndex;
        }

        return ret_value;
    };

    /**
     * @brief Add a progress element to the progress list.
     * @param[in] newProgressElement new element to add
     * @param[0ut] listIndex filled with the list index for the new element, if successful
     * @return true if successful
     * 
     * @note caller MUST guarantee aProgressElement is a valid, unique progress element
     * @note this function SHOULD double check that the added element won't exceed the maximum list size
     */
    virtual bool AddProgressElement(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) = 0;

    /**
     * @brief Modify a progress element in the supported locations list.
     * @param[in] listIndex list index of the location being modified
     * @param[in] modifedProgressElement modified element's contents
     * @return true if successful
     * 
     * @note caller MUST guarantee modified location is a valid, unique location
     * @note this function SHOULD double check that modifedLocation's locationID matches the object at listIndex
     */
    virtual bool ModifyProgressElement(uint32_t listIndex, const Structs::ProgressStruct::Type & modifedProgressElement) = 0;

    /**
     * @brief Clear the progress list
     * @return true if progress was not already null
     * @note no notifications or other side effects
     */
    virtual bool ClearProgress() = 0;
};


} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
