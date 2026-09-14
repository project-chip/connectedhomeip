/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <vector>

namespace chip::app::Clusters::ServiceArea {

// Minimal, non-persistent StorageDelegate: supported/selected areas, supported maps, and progress
// elements are all held in-memory for the lifetime of the process and are lost on restart. It
// starts empty; it does not pre-populate any hardcoded areas or maps on construction. A
// ServiceAreaCluster is constructed with a reference to an instance of this class and drives it
// entirely through the Get*/Add*Raw/Modify*Raw/Remove*Raw calls below; callers that want default
// content (e.g. SimulatedRoboticVacuumCleaner::Init()) populate it explicitly after construction.
class LoggingServiceAreaStorageDelegate : public StorageDelegate
{
public:
    LoggingServiceAreaStorageDelegate() = default;

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

    bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & supportedMap) override;

    bool GetSupportedMapById(uint32_t aMapId, uint32_t & listIndex, MapStructureWrapper & supportedMap) override;

    bool AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex) override;

    bool ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap) override;

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

    bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement) override;

    bool GetProgressElementById(uint32_t aAreaId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement) override;

    bool AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override;

    bool ModifyProgressElementRaw(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) override;

    bool ClearProgressRaw() override;

    bool RemoveProgressElementRaw(uint32_t areaId) override;

private:
    std::vector<AreaStructureWrapper> mSupportedAreas;
    std::vector<MapStructureWrapper> mSupportedMaps;
    std::vector<uint32_t> mSelectedAreas;
    std::vector<Structs::ProgressStruct::Type> mProgressList;
};

} // namespace chip::app::Clusters::ServiceArea
