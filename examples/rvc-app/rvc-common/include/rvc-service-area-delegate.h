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

#include <app/clusters/service-area-server/service-area-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice;

namespace ServiceArea {

class RvcServiceAreaDelegate : public Delegate
{
private:
    // containers for array attributes.
    std::vector<ServiceArea::LocationStructureWrapper> mSupportedLocations;
    std::vector<ServiceArea::MapStructureWrapper> mSupportedMaps;
    std::vector<uint32_t> mSelectedLocations;
    std::vector<ServiceArea::Structs::ProgressStruct::Type> mProgressList;

public:
    CHIP_ERROR Init() override;

    // command support
    bool IsSetSelectedLocationsAllowed(MutableCharSpan statusText) override;

    bool IsValidSelectLocationsSet(const ServiceArea::Commands::SelectLocations::DecodableType & req,
                                   ServiceArea::SelectLocationsStatus & locationStatus, MutableCharSpan statusText) override;

    bool HandleSkipCurrentLocation(MutableCharSpan skipStatusText) override;

    //*************************************************************************
    // Supported Locations accessors

    bool IsSupportedLocationsChangeAllowed() override;

    uint32_t GetNumberOfSupportedLocations() override;

    bool GetSupportedLocationByIndex(uint32_t listIndex, ServiceArea::LocationStructureWrapper & supportedLocation) override;

    bool GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex,
                                  ServiceArea::LocationStructureWrapper & supportedLocation) override;

    bool AddSupportedLocation(const ServiceArea::LocationStructureWrapper & newLocation, uint32_t & listIndex) override;

    bool ModifySupportedLocation(uint32_t listIndex, const ServiceArea::LocationStructureWrapper & modifiedLocation) override;

    bool ClearSupportedLocations() override;

    //*************************************************************************
    // Supported Maps accessors

    bool IsSupportedMapChangeAllowed() override;

    uint32_t GetNumberOfSupportedMaps() override;

    bool GetSupportedMapByIndex(uint32_t listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool AddSupportedMap(const ServiceArea::MapStructureWrapper & newMap, uint32_t & listIndex) override;

    bool ModifySupportedMap(uint32_t listIndex, const ServiceArea::MapStructureWrapper & newMap) override;

    bool ClearSupportedMaps() override;

    //*************************************************************************
    // Selected Locations accessors

    uint32_t GetNumberOfSelectedLocations() override;

    bool GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & selectedLocation) override;

    // IsSelectedLocation() no override

    bool AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex) override;

    bool ClearSelectedLocations() override;

    //*************************************************************************
    // Progress accessors

    uint32_t GetNumberOfProgressElements() override;

    bool GetProgressElementByIndex(uint32_t listIndex, ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex,
                                ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool AddProgressElement(const ServiceArea::Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override;

    bool ModifyProgressElement(uint32_t listIndex,
                               const ServiceArea::Structs::ProgressStruct::Type & modifiedProgressElement) override;

    bool ClearProgress() override;
};

} // namespace ServiceArea

} // namespace Clusters
} // namespace app
} // namespace chip
