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

#include <app/clusters/service-area-server/service-area-storage-delegate.h>
#include <app/util/config.h>
#include <cstring>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice;

namespace ServiceArea {

class RvcServiceAreaStorageDelegate : public StorageDelegate
{
private:
    // containers for array attributes.
    std::vector<ServiceArea::AreaStructureWrapper> mSupportedAreas;
    std::vector<ServiceArea::MapStructureWrapper> mSupportedMaps;
    std::vector<uint32_t> mSelectedAreas;
    std::vector<ServiceArea::Structs::ProgressStruct::Type> mProgressList;

public:
    //*************************************************************************
    // Supported Areas accessors

    uint32_t GetNumberOfSupportedAreas() override;

    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & supportedArea) override;

    bool GetSupportedAreaById(uint32_t aAreaId, uint32_t & listIndex, AreaStructureWrapper & supportedArea) override;

    bool AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex) override;

    bool ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea) override;

    bool ClearSupportedAreasRaw() override;

    bool RemoveSupportedAreaRaw(uint32_t areaId) override;

    //*************************************************************************
    // Supported Maps accessors

    uint32_t GetNumberOfSupportedMaps() override;

    bool GetSupportedMapByIndex(uint32_t listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex) override;

    bool ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & newMap) override;

    bool ClearSupportedMapsRaw() override;

    bool RemoveSupportedMapRaw(uint32_t mapId) override;

    //*************************************************************************
    // Selected Areas accessors

    uint32_t GetNumberOfSelectedAreas() override;

    bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea) override;

    bool AddSelectedAreaRaw(uint32_t aAreaId, uint32_t & listIndex) override;

    bool ClearSelectedAreasRaw() override;

    bool RemoveSelectedAreasRaw(uint32_t areaId) override;

    //*************************************************************************
    // Progress accessors

    uint32_t GetNumberOfProgressElements() override;

    bool GetProgressElementByIndex(uint32_t listIndex, ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex,
                                ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override;

    bool ModifyProgressElementRaw(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) override;

    bool ClearProgressRaw() override;

    bool RemoveProgressElementRaw(uint32_t areaId) override;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
