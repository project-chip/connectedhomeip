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

#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/ServiceArea/Attributes.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>
#include <vector>

#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {
namespace Testing {

using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::ServiceArea;
using namespace chip::Testing;

inline constexpr EndpointId kTestEndpoint = 1;

class InMemoryStorageDelegate : public StorageDelegate
{
public:
    uint32_t GetNumberOfSupportedAreas() override { return static_cast<uint32_t>(mSupportedAreas.size()); }

    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea) override
    {
        if (listIndex >= mSupportedAreas.size())
        {
            return false;
        }
        aSupportedArea = mSupportedAreas[listIndex];
        return true;
    }

    bool AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex) override
    {
        if (mSupportedAreas.size() >= kMaxNumSupportedAreas)
        {
            return false;
        }
        mSupportedAreas.push_back(newArea);
        listIndex = static_cast<uint32_t>(mSupportedAreas.size() - 1);
        return true;
    }

    bool ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea) override
    {
        if (listIndex >= mSupportedAreas.size() || modifiedArea.areaID != mSupportedAreas[listIndex].areaID)
        {
            return false;
        }
        mSupportedAreas[listIndex] = modifiedArea;
        return true;
    }

    bool ClearSupportedAreasRaw() override
    {
        if (mSupportedAreas.empty())
        {
            return false;
        }
        mSupportedAreas.clear();
        return true;
    }

    bool RemoveSupportedAreaRaw(uint32_t areaId) override
    {
        for (auto it = mSupportedAreas.begin(); it != mSupportedAreas.end(); ++it)
        {
            if (it->areaID == areaId)
            {
                mSupportedAreas.erase(it);
                return true;
            }
        }
        return false;
    }

    uint32_t GetNumberOfSupportedMaps() override { return static_cast<uint32_t>(mSupportedMaps.size()); }

    bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap) override
    {
        if (listIndex >= mSupportedMaps.size())
        {
            return false;
        }
        aSupportedMap = mSupportedMaps[listIndex];
        return true;
    }

    bool AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex) override
    {
        if (mSupportedMaps.size() >= kMaxNumSupportedMaps)
        {
            return false;
        }
        mSupportedMaps.push_back(newMap);
        listIndex = static_cast<uint32_t>(mSupportedMaps.size() - 1);
        return true;
    }

    bool ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap) override
    {
        if (listIndex >= mSupportedMaps.size() || modifiedMap.mapID != mSupportedMaps[listIndex].mapID)
        {
            return false;
        }
        mSupportedMaps[listIndex] = modifiedMap;
        return true;
    }

    bool ClearSupportedMapsRaw() override
    {
        if (mSupportedMaps.empty())
        {
            return false;
        }
        mSupportedMaps.clear();
        return true;
    }

    bool RemoveSupportedMapRaw(uint32_t mapId) override
    {
        for (auto it = mSupportedMaps.begin(); it != mSupportedMaps.end(); ++it)
        {
            if (it->mapID == mapId)
            {
                mSupportedMaps.erase(it);
                return true;
            }
        }
        return false;
    }

    uint32_t GetNumberOfSelectedAreas() override { return static_cast<uint32_t>(mSelectedAreas.size()); }

    bool GetSelectedAreaByIndex(uint32_t listIndex, uint32_t & selectedArea) override
    {
        if (listIndex >= mSelectedAreas.size())
        {
            return false;
        }
        selectedArea = mSelectedAreas[listIndex];
        return true;
    }

    bool AddSelectedAreaRaw(uint32_t aAreaId, uint32_t & listIndex) override
    {
        if (mSelectedAreas.size() >= kMaxNumSelectedAreas)
        {
            return false;
        }
        mSelectedAreas.push_back(aAreaId);
        listIndex = static_cast<uint32_t>(mSelectedAreas.size() - 1);
        return true;
    }

    bool ClearSelectedAreasRaw() override
    {
        if (mSelectedAreas.empty())
        {
            return false;
        }
        mSelectedAreas.clear();
        return true;
    }

    bool RemoveSelectedAreasRaw(uint32_t areaId) override
    {
        for (auto it = mSelectedAreas.begin(); it != mSelectedAreas.end(); ++it)
        {
            if (*it == areaId)
            {
                mSelectedAreas.erase(it);
                return true;
            }
        }
        return false;
    }

    uint32_t GetNumberOfProgressElements() override { return static_cast<uint32_t>(mProgress.size()); }

    bool GetProgressElementByIndex(uint32_t listIndex, Structs::ProgressStruct::Type & aProgressElement) override
    {
        if (listIndex >= mProgress.size())
        {
            return false;
        }
        aProgressElement = mProgress[listIndex];
        return true;
    }

    bool AddProgressElementRaw(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override
    {
        if (mProgress.size() >= kMaxNumProgressElements)
        {
            return false;
        }
        mProgress.push_back(newProgressElement);
        listIndex = static_cast<uint32_t>(mProgress.size() - 1);
        return true;
    }

    bool ModifyProgressElementRaw(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) override
    {
        if (listIndex >= mProgress.size() || modifiedProgressElement.areaID != mProgress[listIndex].areaID)
        {
            return false;
        }
        mProgress[listIndex] = modifiedProgressElement;
        return true;
    }

    bool ClearProgressRaw() override
    {
        if (mProgress.empty())
        {
            return false;
        }
        mProgress.clear();
        return true;
    }

    bool RemoveProgressElementRaw(uint32_t areaId) override
    {
        for (auto it = mProgress.begin(); it != mProgress.end(); ++it)
        {
            if (it->areaID == areaId)
            {
                mProgress.erase(it);
                return true;
            }
        }
        return false;
    }

    std::vector<AreaStructureWrapper> mSupportedAreas;
    std::vector<MapStructureWrapper> mSupportedMaps;
    std::vector<uint32_t> mSelectedAreas;
    std::vector<Structs::ProgressStruct::Type> mProgress;
};

class MockDelegate : public Delegate
{
public:
    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override
    {
        if (!mSetSelectedAreasAllowed)
        {
            CopyCharSpanToMutableCharSpanWithTruncation(mInvalidModeText, statusText);
        }
        return mSetSelectedAreasAllowed;
    }

    bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, SelectAreasStatus & locationStatus,
                               MutableCharSpan & statusText) override
    {
        locationStatus = mSelectAreasSetStatus;
        if (locationStatus != SelectAreasStatus::kSuccess)
        {
            CopyCharSpanToMutableCharSpanWithTruncation(mSelectAreasSetStatusText, statusText);
            return false;
        }
        (void) selectedAreas;
        return true;
    }

    bool HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText) override
    {
        if (!mSkipAreaAllowed)
        {
            CopyCharSpanToMutableCharSpanWithTruncation(mSkipAreaStatusText, skipStatusText);
            return false;
        }
        mLastSkippedArea = skippedArea;
        return true;
    }

    bool IsSupportedAreasChangeAllowed() override { return mSupportedAreasChangeAllowed; }

    bool IsSupportedMapChangeAllowed() override { return mSupportedMapChangeAllowed; }

    bool mSetSelectedAreasAllowed           = true;
    bool mSkipAreaAllowed                   = true;
    bool mSupportedAreasChangeAllowed       = true;
    bool mSupportedMapChangeAllowed         = true;
    SelectAreasStatus mSelectAreasSetStatus = SelectAreasStatus::kSuccess;
    CharSpan mInvalidModeText               = "device not idle"_span;
    CharSpan mSelectAreasSetStatusText      = "invalid set"_span;
    CharSpan mSkipAreaStatusText            = "cannot skip in current mode"_span;
    uint32_t mLastSkippedArea               = 0;

    void Reset()
    {
        mSetSelectedAreasAllowed     = true;
        mSkipAreaAllowed             = true;
        mSupportedAreasChangeAllowed = true;
        mSupportedMapChangeAllowed   = true;
        mSelectAreasSetStatus        = SelectAreasStatus::kSuccess;
        mInvalidModeText             = "device not idle"_span;
        mSelectAreasSetStatusText    = "invalid set"_span;
        mSkipAreaStatusText          = "cannot skip in current mode"_span;
        mLastSkippedArea             = 0;
    }
};

struct ClusterConfig
{
    BitMask<Feature> features;
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
};

inline ServiceAreaCluster::OptionalAttributeSet AllOptionalAttributes()
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::SupportedMaps::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    return optionalAttributes;
}

inline AreaStructureWrapper MakeNamedArea(uint32_t areaId, const char * name)
{
    AreaStructureWrapper area;
    CharSpan nameSpan(name, strlen(name));
    area.SetAreaId(areaId)
        .SetMapId(DataModel::NullNullable)
        .SetLocationInfo(nameSpan, DataModel::NullNullable, DataModel::NullNullable);
    return area;
}

inline AreaStructureWrapper
MakeLandmarkOnlyArea(uint32_t areaId, LandmarkTag landmark,
                     const DataModel::Nullable<RelativePositionTag> & position = DataModel::NullNullable)
{
    AreaStructureWrapper area;
    area.SetAreaId(areaId).SetMapId(DataModel::NullNullable).SetLocationInfoNull().SetLandmarkInfo(landmark, position);
    return area;
}

inline AreaStructureWrapper MakeFloorArea(uint32_t areaId, int16_t floor)
{
    AreaStructureWrapper area;
    area.SetAreaId(areaId)
        .SetMapId(DataModel::NullNullable)
        .SetLocationInfo(""_span, DataModel::MakeNullable(floor), DataModel::NullNullable);
    return area;
}

inline AreaStructureWrapper MakeMappedArea(uint32_t areaId, uint32_t mapId, const char * name)
{
    AreaStructureWrapper area;
    CharSpan nameSpan(name, strlen(name));
    area.SetAreaId(areaId)
        .SetMapId(DataModel::MakeNullable(mapId))
        .SetLocationInfo(nameSpan, DataModel::NullNullable, DataModel::NullNullable);
    return area;
}

inline bool AddSupportedArea(ServiceAreaCluster & cluster, AreaStructureWrapper area)
{
    return cluster.AddSupportedArea(area);
}

inline bool ModifySupportedArea(ServiceAreaCluster & cluster, AreaStructureWrapper area)
{
    return cluster.ModifySupportedArea(area);
}

inline bool AddSelectedArea(ServiceAreaCluster & cluster, uint32_t areaId)
{
    return cluster.AddSelectedArea(areaId);
}

inline Commands::SelectAreas::Type MakeSelectAreasRequest(std::initializer_list<uint32_t> areaIds)
{
    Commands::SelectAreas::Type request;
    static uint32_t sAreaIdBuffer[kMaxNumSelectedAreas];
    size_t index = 0;
    for (uint32_t id : areaIds)
    {
        sAreaIdBuffer[index++] = id;
    }
    request.newAreas = DataModel::List<const uint32_t>(sAreaIdBuffer, index);
    return request;
}

inline std::vector<uint32_t> ReadSelectedAreasList(ClusterTester & tester)
{
    std::vector<uint32_t> result;
    DataModel::DecodableList<uint32_t> selected;
    if (!tester.ReadAttribute(Attributes::SelectedAreas::Id, selected).IsSuccess())
    {
        return result;
    }

    auto it = selected.begin();
    while (it.Next())
    {
        result.push_back(it.GetValue());
    }
    return result;
}

inline size_t CountSupportedAreas(ClusterTester & tester)
{
    DataModel::DecodableList<Structs::AreaStruct::DecodableType> areas;
    if (!tester.ReadAttribute(Attributes::SupportedAreas::Id, areas).IsSuccess())
    {
        return 0;
    }

    size_t count = 0;
    auto it      = areas.begin();
    while (it.Next())
    {
        ++count;
    }
    return count;
}

inline size_t CountSupportedMaps(ClusterTester & tester)
{
    DataModel::DecodableList<Structs::MapStruct::DecodableType> maps;
    if (!tester.ReadAttribute(Attributes::SupportedMaps::Id, maps).IsSuccess())
    {
        return 0;
    }

    size_t count = 0;
    auto it      = maps.begin();
    while (it.Next())
    {
        ++count;
    }
    return count;
}

inline std::optional<Structs::ProgressStruct::DecodableType> FindProgressForArea(ClusterTester & tester, uint32_t areaId)
{
    DataModel::DecodableList<Structs::ProgressStruct::DecodableType> progressList;
    if (!tester.ReadAttribute(Attributes::Progress::Id, progressList).IsSuccess())
    {
        return std::nullopt;
    }

    auto it = progressList.begin();
    while (it.Next())
    {
        if (it.GetValue().areaID == areaId)
        {
            return it.GetValue();
        }
    }
    return std::nullopt;
}

inline bool DirtyListContainsAttribute(ClusterTester & tester, AttributeId attributeId)
{
    for (const auto & path : tester.GetDirtyList())
    {
        if (path.mAttributeId == attributeId)
        {
            return true;
        }
    }
    return false;
}

class ServiceAreaClusterTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        CreateCluster({});
    }

    void TearDown() override
    {
        if (mCluster)
        {
            mCluster->Shutdown(ClusterShutdownType::kClusterShutdown);
        }
        mTester.reset();
        mCluster.reset();
        chip::Platform::MemoryShutdown();
    }

protected:
    void CreateCluster(const ClusterConfig & config)
    {
        if (mCluster)
        {
            mCluster->Shutdown(ClusterShutdownType::kClusterShutdown);
        }
        mStorage = InMemoryStorageDelegate{};
        mDelegate.Reset();
        mCluster =
            std::make_unique<ServiceAreaCluster>(kTestEndpoint, mStorage, mDelegate, config.features, config.optionalAttributes);
        mTester = std::make_unique<ClusterTester>(*mCluster);
        ASSERT_EQ(mCluster->Startup(mTester->GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void SeedBasicAreas()
    {
        ASSERT_TRUE(AddSupportedArea(*mCluster, MakeNamedArea(0, "yellow bedroom")));
        ASSERT_TRUE(AddSupportedArea(*mCluster, MakeNamedArea(1, "orange bedroom")));
    }

    InMemoryStorageDelegate mStorage;
    MockDelegate mDelegate;
    std::unique_ptr<ServiceAreaCluster> mCluster;
    std::unique_ptr<ClusterTester> mTester;
};

} // namespace Testing
} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
