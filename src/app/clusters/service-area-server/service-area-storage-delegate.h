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

/**
 * ServiceArea::StorageDelegate Defines methods for implementing target/application-specific
 * logic for managing the storage of some Service Area cluster attributes.
 */
class StorageDelegate
{
public:
    StorageDelegate()          = default;
    virtual ~StorageDelegate() = default;

    /**
     * @brief This method will be called during the ServiceArea server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise the memory required.
     */
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; };

    //*************************************************************************
    // Supported Areas accessors and manipulators

    virtual uint32_t GetNumberOfSupportedAreas() = 0;

    /**
     * @brief Get a supported area using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aSupportedArea a copy of the area contents, if found.
     * @return true if an area is found, false otherwise.
     */
    virtual bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea) = 0;

    /**
     * @brief Get a supported area that matches a areaID.
     * @param[in] aAreaId the areaID to search for.
     * @param[out] listIndex the area's index in the list, if found.
     * @param[out] aSupportedArea a copy of the area contents, if found.
     * @return true if an area is found, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & aSupportedArea);

    /**
     * @return true if an area with the aAreaId ID exists in the supported areas attribute. False otherwise.
     */
    virtual bool IsSupportedArea(uint32_t aAreaId);

    /**
     * This method is called by the server instance to add a new area to the list.
     * The server instance will ensure that the newArea is a valid, unique area.
     * @param [in] newArea new area to add.
     * @param [out] listIndex filled with the list index for the new area, if successful.
     * @return true if successful, false otherwise.

     * @note this method SHOULD double check that the added area won't exceed the maximum list size.
     */
    virtual bool AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing area in the list.
     * The server instance will ensure that the modifiedArea is a valid, unique area.
     * @param[in] listIndex The index of the area being modified.
     * @param[in] modifiedArea An area with the modified contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that newArea's areaID matches the object at listIndex.
     */
    virtual bool ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea) = 0;

    /**
     * @return true if supported locations was not already null, false otherwise.
     */
    virtual bool ClearSupportedAreasRaw() = 0;

    /**
     * @param areaId the ID af the area to be removed.
     * @return true if the area was removed, false otherwise.
     */
    virtual bool RemoveSupportedAreaRaw(uint32_t areaId);

    //*************************************************************************
    // Supported Maps accessors and manipulators

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
    virtual bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap);

    /**
     * @return true if a map with the aMapId ID exists in the supported maps attribute. False otherwise.
     */
    bool IsSupportedMap(uint32_t aMapId);

    /**
     * This method is called by the server instance to add a new map to the list.
     * The server instance will ensure that the newMap is a valid, unique map.
     * @param[in] newMap The new map to add.
     * @param[out] listIndex filled with the list index of the new map, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added map won't exceed the maximum list size
     */
    virtual bool AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing map in the list.
     * The server instance will ensure that the modifiedMap is a valid, unique map.
     * @param[in] listIndex The index of the map being modified.
     * @param[in] modifiedMapA map with the modified contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that modifiedMap's mapID matches the object at listIndex.
     */
    virtual bool ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap) = 0;

    /**
     * @return true if supported maps was not already null, false otherwise.
     */
    virtual bool ClearSupportedMapsRaw() = 0;

    /**
     * @param mapId the ID of the map to be removed.
     * @return true if a map is removed. False otherwise.
     */
    virtual bool RemoveSupportedMapRaw(uint32_t mapId);

    //*************************************************************************
    // Selected Areas accessors and manipulators

    virtual uint32_t GetNumberOfSelectedAreas() = 0;

    /**
     * @brief Get a selected area using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] selectedArea the selected area value, if found.
     * @return true if a selected area is found, false otherwise.
     */
    virtual bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea) = 0;

    /**
     * @return true if the aAreaId areaID is found in the SelectedAreas list, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool IsSelectedArea(uint32_t aAreaId);

    /**
     * This method is called by the server instance to add a new selected area to the list.
     * The server instance will ensure that the aAreaId references a SUPPORTED area, and is unique within selected
     * areas.
     * @param[in] aAreaId The new areaID to add.
     * @param[out] listIndex filled with the list index of the new area, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added area won't exceed the maximum list size.
     */
    virtual bool AddSelectedAreaRaw(uint32_t aAreaId, uint32_t & listIndex) = 0;

    /**
     * @return true if selected locations was not already null, false otherwise.
     */
    virtual bool ClearSelectedAreasRaw() = 0;

    /**
     * @param areaId the area ID to be removed from the SelectedAreas attribute.
     * @return ture if this ID was removed, false otherwise.
     */
    virtual bool RemoveSelectedAreasRaw(uint32_t areaId) = 0;

    //*************************************************************************
    // Progress accessors and manipulators

    virtual uint32_t GetNumberOfProgressElements() = 0;

    /**
     * @brief Get a progress element using the position in the list.
     * @param[in] listIndex the position in the list.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     */
    virtual bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement) = 0;

    /**
     * @brief Get a progress element that matches a areaID.
     * @param[in] aAreaId the areaID to search for.
     * @param[out] listIndex the location's index in the list, if found.
     * @param[out] aProgressElement  copy of the progress element contents, if found.
     * @return true if a progress element is found, false otherwise.
     *
     * @note may be overloaded in device implementation for optimization, if desired.
     */
    virtual bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement);

    /**
     * @brief Is the progress element in the progress list?
     * @param[in] aAreaId location id of the progress element.
     * @return true if the progress element identified by Id is in the progress list.
     */
    virtual bool IsProgressElement(uint32_t aAreaId);

    /**
     * This method is called by the server instance to add a new progress element to the list.
     * The server instance will ensure that the newProgressElement is a valid, unique progress element.
     * @param[in] newProgressElement The new element to add.
     * @param[out] listIndex is filled with the list index for the new element, if successful.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that the added element won't exceed the maximum list size.
     */
    virtual bool AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) = 0;

    /**
     * This method is called by the server instance to modify an existing progress element in the list.
     * The server instance will ensure that the modifiedProgressElement is a valid and unique progress element.
     * @param[in] listIndex The list index of the progress element being modified.
     * @param[in] modifiedProgressElement modified element's contents.
     * @return true if successful, false otherwise.
     *
     * @note this function SHOULD double check that modifiedProgressElement's areaID matches the object at listIndex
     */
    virtual bool ModifyProgressElementRaw(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) = 0;

    /**
     * @return true if progress list was not already null, false otherwise.
     */
    virtual bool ClearProgressRaw() = 0;

    /**
     * @param areaId the area ID of the progress element to be removed.
     * @return ture if the progress element was removed, false otherwise.
     */
    virtual bool RemoveProgressElementRaw(uint32_t areaId) = 0;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
