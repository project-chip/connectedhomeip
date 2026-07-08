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

#include <pw_unit_test/framework.h>

#include <app/clusters/service-area-server/service-area-server.h>
#include <app/data-model/Nullable.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

#include <vector>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

namespace {

// Minimal vector-backed StorageDelegate. Only the SupportedAreas / SupportedMaps
// lists are exercised by RemoveSupportedMap; the SelectedAreas and Progress lists
// are intentionally left empty.
class MockStorageDelegate : public StorageDelegate
{
public:
    std::vector<AreaStructureWrapper> supportedAreas;
    std::vector<MapStructureWrapper> supportedMaps;

    bool HasSupportedArea(uint32_t areaId)
    {
        for (auto & area : supportedAreas)
        {
            if (area.areaID == areaId)
            {
                return true;
            }
        }
        return false;
    }

    //*** Supported Areas ***
    uint32_t GetNumberOfSupportedAreas() override { return static_cast<uint32_t>(supportedAreas.size()); }

    bool GetSupportedAreaByIndex(uint32_t listIndex, AreaStructureWrapper & aSupportedArea) override
    {
        if (listIndex >= supportedAreas.size())
        {
            return false;
        }
        aSupportedArea = supportedAreas[listIndex];
        return true;
    }

    bool AddSupportedAreaRaw(const AreaStructureWrapper & newArea, uint32_t & listIndex) override
    {
        supportedAreas.push_back(newArea);
        listIndex = static_cast<uint32_t>(supportedAreas.size() - 1);
        return true;
    }

    bool ModifySupportedAreaRaw(uint32_t listIndex, const AreaStructureWrapper & modifiedArea) override
    {
        if (listIndex >= supportedAreas.size())
        {
            return false;
        }
        supportedAreas[listIndex] = modifiedArea;
        return true;
    }

    bool ClearSupportedAreasRaw() override
    {
        bool wasNotEmpty = !supportedAreas.empty();
        supportedAreas.clear();
        return wasNotEmpty;
    }

    bool RemoveSupportedAreaRaw(uint32_t areaId) override
    {
        for (auto it = supportedAreas.begin(); it != supportedAreas.end(); ++it)
        {
            if (it->areaID == areaId)
            {
                supportedAreas.erase(it);
                return true;
            }
        }
        return false;
    }

    //*** Supported Maps ***
    uint32_t GetNumberOfSupportedMaps() override { return static_cast<uint32_t>(supportedMaps.size()); }

    bool GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap) override
    {
        if (listIndex >= supportedMaps.size())
        {
            return false;
        }
        aSupportedMap = supportedMaps[listIndex];
        return true;
    }

    bool AddSupportedMapRaw(const MapStructureWrapper & newMap, uint32_t & listIndex) override
    {
        supportedMaps.push_back(newMap);
        listIndex = static_cast<uint32_t>(supportedMaps.size() - 1);
        return true;
    }

    bool ModifySupportedMapRaw(uint32_t listIndex, const MapStructureWrapper & modifiedMap) override
    {
        if (listIndex >= supportedMaps.size())
        {
            return false;
        }
        supportedMaps[listIndex] = modifiedMap;
        return true;
    }

    bool ClearSupportedMapsRaw() override
    {
        bool wasNotEmpty = !supportedMaps.empty();
        supportedMaps.clear();
        return wasNotEmpty;
    }

    bool RemoveSupportedMapRaw(uint32_t mapId) override
    {
        for (auto it = supportedMaps.begin(); it != supportedMaps.end(); ++it)
        {
            if (it->mapID == mapId)
            {
                supportedMaps.erase(it);
                return true;
            }
        }
        return false;
    }

    //*** Selected Areas (unused - empty) ***
    uint32_t GetNumberOfSelectedAreas() override { return 0; }
    bool GetSelectedAreaByIndex(uint32_t, uint32_t &) override { return false; }
    bool AddSelectedAreaRaw(uint32_t, uint32_t &) override { return false; }
    bool ClearSelectedAreasRaw() override { return false; }
    bool RemoveSelectedAreasRaw(uint32_t) override { return false; }

    //*** Progress (unused - empty) ***
    uint32_t GetNumberOfProgressElements() override { return 0; }
    bool GetProgressElementByIndex(uint32_t, Structs::ProgressStruct::Type &) override { return false; }
    bool AddProgressElementRaw(const Structs::ProgressStruct::Type &, uint32_t &) override { return false; }
    bool ModifyProgressElementRaw(uint32_t, const Structs::ProgressStruct::Type &) override { return false; }
    bool ClearProgressRaw() override { return false; }
    bool RemoveProgressElementRaw(uint32_t) override { return false; }
};

// Permissive delegate - allows every mutation.
class MockDelegate : public Delegate
{
public:
    bool IsSetSelectedAreasAllowed(MutableCharSpan &) override { return true; }
    bool IsValidSelectAreasSet(const Span<const uint32_t> &, SelectAreasStatus &, MutableCharSpan &) override { return true; }
    bool IsSupportedAreasChangeAllowed() override { return true; }
    bool IsSupportedMapChangeAllowed() override { return true; }
};

AreaStructureWrapper MakeArea(uint32_t areaId, uint32_t mapId)
{
    AreaStructureWrapper area;
    area.SetAreaId(areaId).SetMapId(chip::app::DataModel::MakeNullable<uint32_t>(mapId));
    return area;
}

void AddArea(MockStorageDelegate & storage, uint32_t areaId, uint32_t mapId)
{
    uint32_t ignoredIndex = 0;
    storage.AddSupportedAreaRaw(MakeArea(areaId, mapId), ignoredIndex);
}

} // namespace

// Regression test for the RemoveSupportedMap packed-buffer index bug.
//
// RemoveSupportedMap collects the areaIDs whose mapID matches the removed map
// into a temporary stack buffer, then removes each. The write into that buffer
// must use the *packed* cursor (count of matches so far), not the *source* cursor
// (index over all supported areas). When some supported areas belong to a
// different map, writing at the source cursor scatters the matches to their
// source slots; the subsequent reduce_size(<match count>) then keeps only the
// front of the buffer, so any match whose source index is >= the match count is
// dropped (never removed), and the gaps feed uninitialized stack values into
// RemoveSupportedAreaRaw.
//
// The layout leads with a non-matching entry so the drift starts at index 0:
// areas 100/101/102 belong to map 1 (source indices 1, 3, 4), areas 200/201 to
// map 2. There are 3 matches, so matches at source index >= 3 (areas 101 and 102)
// are dropped by the bug. After the fix all three map-1 areas are removed and the
// two map-2 areas are kept.
TEST(TestServiceAreaCluster, RemoveSupportedMapRemovesAllMatchingAreasWhenSparse)
{
    MockStorageDelegate storage;
    MockDelegate delegate;

    storage.supportedMaps.push_back(MapStructureWrapper(1, "MapOne"_span));
    storage.supportedMaps.push_back(MapStructureWrapper(2, "MapTwo"_span));

    AddArea(storage, 200, 2); // index 0: non-match -> buf[0] never written
    AddArea(storage, 100, 1); // index 1: match, source index 1 < 3 -> removed even when buggy
    AddArea(storage, 201, 2); // index 2: non-match
    AddArea(storage, 101, 1); // index 3: match, source index 3 >= 3 -> dropped by the bug
    AddArea(storage, 102, 1); // index 4: match, source index 4 >= 3 -> dropped by the bug

    Instance instance(&storage, &delegate, /* endpoint */ 1, BitMask<ServiceArea::Feature>());

    EXPECT_TRUE(instance.RemoveSupportedMap(1));

    // Every area that referenced the removed map must be gone.
    EXPECT_FALSE(storage.HasSupportedArea(100));
    EXPECT_FALSE(storage.HasSupportedArea(101)); // dropped by the bug
    EXPECT_FALSE(storage.HasSupportedArea(102)); // dropped by the bug

    // Areas belonging to the surviving map must be untouched.
    EXPECT_TRUE(storage.HasSupportedArea(200));
    EXPECT_TRUE(storage.HasSupportedArea(201));

    EXPECT_EQ(storage.GetNumberOfSupportedAreas(), 2u);
}
