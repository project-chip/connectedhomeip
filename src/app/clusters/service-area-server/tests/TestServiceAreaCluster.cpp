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

#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/ServiceArea/Attributes.h>
#include <clusters/ServiceArea/Metadata.h>
#include <clusters/shared/Attributes.h>
#include <lib/core/CHIPError.h>
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
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr EndpointId kTestEndpoint = 1;

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

ServiceAreaCluster::OptionalAttributeSet AllOptionalAttributes()
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::SupportedMaps::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    return optionalAttributes;
}

ClusterConfig ClusterConfigWithCurrentArea()
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    return { .optionalAttributes = optionalAttributes };
}

ClusterConfig ClusterConfigWithProgress()
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    return { .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes };
}

AreaStructureWrapper MakeNamedArea(uint32_t areaId, const char * name)
{
    AreaStructureWrapper area;
    CharSpan nameSpan(name, strlen(name));
    area.SetAreaId(areaId)
        .SetMapId(DataModel::NullNullable)
        .SetLocationInfo(nameSpan, DataModel::NullNullable, DataModel::NullNullable);
    return area;
}

AreaStructureWrapper
MakeLandmarkOnlyArea(uint32_t areaId, Globals::LandmarkTag landmark,
                     const DataModel::Nullable<Globals::RelativePositionTag> & position = DataModel::NullNullable)
{
    AreaStructureWrapper area;
    area.SetAreaId(areaId).SetMapId(DataModel::NullNullable).SetLocationInfoNull().SetLandmarkInfo(landmark, position);
    return area;
}

AreaStructureWrapper MakeFloorArea(uint32_t areaId, int16_t floor)
{
    AreaStructureWrapper area;
    area.SetAreaId(areaId)
        .SetMapId(DataModel::NullNullable)
        .SetLocationInfo(""_span, DataModel::MakeNullable(floor), DataModel::NullNullable);
    return area;
}

AreaStructureWrapper MakeMappedArea(uint32_t areaId, uint32_t mapId, const char * name)
{
    AreaStructureWrapper area;
    CharSpan nameSpan(name, strlen(name));
    area.SetAreaId(areaId)
        .SetMapId(DataModel::MakeNullable(mapId))
        .SetLocationInfo(nameSpan, DataModel::NullNullable, DataModel::NullNullable);
    return area;
}

bool AddSupportedArea(ServiceAreaCluster & cluster, AreaStructureWrapper area)
{
    return cluster.AddSupportedArea(area);
}

bool ModifySupportedArea(ServiceAreaCluster & cluster, AreaStructureWrapper area)
{
    return cluster.ModifySupportedArea(area);
}

bool AddSelectedArea(ServiceAreaCluster & cluster, uint32_t areaId)
{
    return cluster.AddSelectedArea(areaId);
}

Commands::SelectAreas::Type MakeSelectAreasRequest(std::initializer_list<uint32_t> areaIds)
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

std::vector<uint32_t> ReadSelectedAreasList(chip::Testing::ClusterTester & tester)
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

size_t CountSupportedAreas(chip::Testing::ClusterTester & tester)
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

std::vector<uint32_t> ReadSupportedAreaIds(chip::Testing::ClusterTester & tester)
{
    std::vector<uint32_t> result;
    DataModel::DecodableList<Structs::AreaStruct::DecodableType> areas;
    if (!tester.ReadAttribute(Attributes::SupportedAreas::Id, areas).IsSuccess())
    {
        return result;
    }

    auto it = areas.begin();
    while (it.Next())
    {
        result.push_back(it.GetValue().areaID);
    }
    return result;
}

bool Contains(const std::vector<uint32_t> & ids, uint32_t id)
{
    for (uint32_t entry : ids)
    {
        if (entry == id)
        {
            return true;
        }
    }
    return false;
}

size_t CountSupportedMaps(chip::Testing::ClusterTester & tester)
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

std::optional<Structs::ProgressStruct::DecodableType> FindProgressForArea(chip::Testing::ClusterTester & tester, uint32_t areaId)
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

// Helpers to access invoke results and progress elements without unchecked optional access.
template <typename ResponseType, typename StatusEnum>
std::optional<StatusEnum> InvokeResponseStatus(const chip::Testing::ClusterTester::InvokeResult<ResponseType> & result)
{
    VerifyOrReturnValue(result.response.has_value(), std::nullopt);
    return result.response->status;
}

std::optional<SelectAreasStatus>
SelectAreasResponseStatus(const chip::Testing::ClusterTester::InvokeResult<Commands::SelectAreasResponse::DecodableType> & result)
{
    return InvokeResponseStatus<Commands::SelectAreasResponse::DecodableType, SelectAreasStatus>(result);
}

std::optional<SkipAreaStatus>
SkipAreaResponseStatus(const chip::Testing::ClusterTester::InvokeResult<Commands::SkipAreaResponse::DecodableType> & result)
{
    return InvokeResponseStatus<Commands::SkipAreaResponse::DecodableType, SkipAreaStatus>(result);
}

template <typename ResponseType>
bool InvokeResponseHasNonEmptyStatusText(const chip::Testing::ClusterTester::InvokeResult<ResponseType> & result)
{
    VerifyOrReturnValue(result.response.has_value(), false);
    return !result.response->statusText.empty();
}

std::optional<OperationalStatusEnum> ProgressElementStatus(const std::optional<Structs::ProgressStruct::DecodableType> & progress)
{
    VerifyOrReturnValue(progress.has_value(), std::nullopt);
    return progress->status;
}

std::optional<bool> ProgressElementHasTotalOperationalTime(const std::optional<Structs::ProgressStruct::DecodableType> & progress)
{
    VerifyOrReturnValue(progress.has_value(), std::nullopt);
    return progress->totalOperationalTime.HasValue();
}

std::optional<uint32_t>
ProgressElementTotalOperationalTimeSeconds(const std::optional<Structs::ProgressStruct::DecodableType> & progress)
{
    VerifyOrReturnValue(progress.has_value(), std::nullopt);
    VerifyOrReturnValue(progress->totalOperationalTime.HasValue(), std::nullopt);
    return progress->totalOperationalTime.Value().Value();
}

bool DirtyListContainsAttribute(chip::Testing::ClusterTester & tester, AttributeId attributeId)
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
        mTester = std::make_unique<chip::Testing::ClusterTester>(*mCluster);
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
    std::unique_ptr<chip::Testing::ClusterTester> mTester;
};

} // namespace

//*****************************************************************************
// cluster metadata

TEST_F(ServiceAreaClusterTest, ClusterRevisionMatchesSpec)
{
    uint16_t revision = 0;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::ClusterRevision::Id, revision).IsSuccess());
    EXPECT_EQ(revision, static_cast<uint16_t>(kRevision));
}

TEST_F(ServiceAreaClusterTest, FeatureMapReflectsEnabledFeatures)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kSelectWhileRunning, Feature::kProgressReporting, Feature::kMaps) });

    uint32_t featureMap = 0;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::FeatureMap::Id, featureMap).IsSuccess());
    EXPECT_EQ(featureMap, 0x07u);
}

TEST_F(ServiceAreaClusterTest, MandatoryAttributesAreAdvertised)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(mCluster->Attributes(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    auto HasAttr = [&](AttributeId id) {
        for (const auto & entry : list)
        {
            if (entry.attributeId == id)
            {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(HasAttr(Attributes::ClusterRevision::Id));
    EXPECT_TRUE(HasAttr(Attributes::FeatureMap::Id));
    EXPECT_TRUE(HasAttr(Attributes::SupportedAreas::Id));
    EXPECT_TRUE(HasAttr(Attributes::SelectedAreas::Id));
    EXPECT_FALSE(HasAttr(Attributes::SupportedMaps::Id));
    EXPECT_FALSE(HasAttr(Attributes::CurrentArea::Id));
    EXPECT_FALSE(HasAttr(Attributes::EstimatedEndTime::Id));
    EXPECT_FALSE(HasAttr(Attributes::Progress::Id));
}

TEST_F(ServiceAreaClusterTest, OptionalAttributesAreAdvertisedWhenEnabled)
{
    CreateCluster({ .features           = BitMask<Feature>(Feature::kMaps, Feature::kProgressReporting),
                    .optionalAttributes = AllOptionalAttributes() });

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(mCluster->Attributes(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    auto HasAttr = [&](AttributeId id) {
        for (const auto & entry : list)
        {
            if (entry.attributeId == id)
            {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(HasAttr(Attributes::SupportedMaps::Id));
    EXPECT_TRUE(HasAttr(Attributes::CurrentArea::Id));
    EXPECT_TRUE(HasAttr(Attributes::EstimatedEndTime::Id));
    EXPECT_TRUE(HasAttr(Attributes::Progress::Id));
}

TEST_F(ServiceAreaClusterTest, SelectAreasCommandIsAlwaysAccepted)
{
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mCluster->AcceptedCommands(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    bool hasSelectAreas = false;
    bool hasSkipArea    = false;
    for (const auto & entry : list)
    {
        if (entry.commandId == Commands::SelectAreas::Id)
        {
            hasSelectAreas = true;
        }
        if (entry.commandId == Commands::SkipArea::Id)
        {
            hasSkipArea = true;
        }
    }
    EXPECT_TRUE(hasSelectAreas);
    EXPECT_FALSE(hasSkipArea);
}

TEST_F(ServiceAreaClusterTest, SkipAreaCommandIsAcceptedWhenCurrentAreaIsEnabled)
{
    CreateCluster(ClusterConfigWithCurrentArea());

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mCluster->AcceptedCommands(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    bool hasSelectAreas = false;
    bool hasSkipArea    = false;
    for (const auto & entry : list)
    {
        if (entry.commandId == Commands::SelectAreas::Id)
        {
            hasSelectAreas = true;
        }
        if (entry.commandId == Commands::SkipArea::Id)
        {
            hasSkipArea = true;
        }
    }
    EXPECT_TRUE(hasSelectAreas);
    EXPECT_TRUE(hasSkipArea);
}

TEST_F(ServiceAreaClusterTest, SkipAreaCommandIsAcceptedWhenProgressIsEnabled)
{
    CreateCluster(ClusterConfigWithProgress());

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mCluster->AcceptedCommands(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    bool hasSelectAreas = false;
    bool hasSkipArea    = false;
    for (const auto & entry : list)
    {
        if (entry.commandId == Commands::SelectAreas::Id)
        {
            hasSelectAreas = true;
        }
        if (entry.commandId == Commands::SkipArea::Id)
        {
            hasSkipArea = true;
        }
    }
    EXPECT_TRUE(hasSelectAreas);
    EXPECT_TRUE(hasSkipArea);
}

//*****************************************************************************
// SupportedAreas attribute

TEST_F(ServiceAreaClusterTest, SupportedAreasStartsEmpty)
{
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

TEST_F(ServiceAreaClusterTest, MeaningfulAreaDescriptionsAppearInSupportedAreas)
{
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeNamedArea(0, "blue room")));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeFloorArea(1, 2)));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeLandmarkOnlyArea(2, LandmarkTag::kTable)));

    EXPECT_EQ(CountSupportedAreas(*mTester), 3u);
}

TEST_F(ServiceAreaClusterTest, InvalidAreaDescriptionsDoNotAppearInSupportedAreas)
{
    AreaStructureWrapper missingDescription;
    missingDescription.SetAreaId(0).SetMapId(DataModel::NullNullable).SetLocationInfoNull().SetLandmarkInfoNull();
    AddSupportedArea(*mCluster, missingDescription);
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);

    AreaStructureWrapper emptySemantics;
    emptySemantics.SetAreaId(1)
        .SetMapId(DataModel::NullNullable)
        .SetLocationInfo(""_span, DataModel::NullNullable, DataModel::NullNullable)
        .SetLandmarkInfoNull();
    AddSupportedArea(*mCluster, emptySemantics);
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

TEST_F(ServiceAreaClusterTest, DuplicateAreaIdsAreNotAddedToSupportedAreas)
{
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeNamedArea(0, "room a")));
    AddSupportedArea(*mCluster, MakeNamedArea(0, "room b"));
    EXPECT_EQ(CountSupportedAreas(*mTester), 1u);
}

TEST_F(ServiceAreaClusterTest, DuplicateAreaInfoIsNotAddedWhenMapsAreEmpty)
{
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeNamedArea(0, "hallway")));
    AddSupportedArea(*mCluster, MakeNamedArea(1, "hallway"));
    EXPECT_EQ(CountSupportedAreas(*mTester), 1u);
}

TEST_F(ServiceAreaClusterTest, SameAreaInfoOnDifferentMapsIsAllowed)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "floor one"_span));
    ASSERT_TRUE(mCluster->AddSupportedMap(2, "floor two"_span));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(5, 1, "hallway")));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(3, 2, "hallway")));

    EXPECT_EQ(CountSupportedAreas(*mTester), 2u);
}

TEST_F(ServiceAreaClusterTest, AreasWithoutMapsMustHaveNullMapId)
{
    AddSupportedArea(*mCluster, MakeMappedArea(0, 1, "room"));
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

TEST_F(ServiceAreaClusterTest, AreasMustReferenceKnownMapsWhenMapsExist)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "main floor"_span));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(0, 1, "kitchen")));
    AddSupportedArea(*mCluster, MakeMappedArea(1, 99, "garage"));

    EXPECT_EQ(CountSupportedAreas(*mTester), 1u);
}

TEST_F(ServiceAreaClusterTest, RemovingSupportedAreaUpdatesSelectedAreasAndCurrentArea)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(0)));
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(1)));
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(1))));

    ASSERT_TRUE(mCluster->RemoveSupportedArea(1));

    auto selected = ReadSelectedAreasList(*mTester);
    ASSERT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected[0], 0u);

    DataModel::Nullable<uint32_t> currentArea;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::CurrentArea::Id, currentArea).IsSuccess());
    EXPECT_TRUE(currentArea.IsNull());
}

TEST_F(ServiceAreaClusterTest, ClearingSupportedAreasClearsDependentAttributes)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(0)));
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));

    ASSERT_TRUE(mCluster->ClearSupportedAreas());

    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());

    DataModel::Nullable<uint32_t> currentArea;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::CurrentArea::Id, currentArea).IsSuccess());
    EXPECT_TRUE(currentArea.IsNull());

    DataModel::DecodableList<Structs::ProgressStruct::DecodableType> progress;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::Progress::Id, progress).IsSuccess());
    EXPECT_FALSE(progress.begin().Next());
}

TEST_F(ServiceAreaClusterTest, SupportedAreasListIsUnchangedWhenDelegateDisallowsUpdates)
{
    mDelegate.mSupportedAreasChangeAllowed = false;
    AddSupportedArea(*mCluster, MakeNamedArea(0, "room"));
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

TEST_F(ServiceAreaClusterTest, ModifyingSupportedAreaUpdatesReadableAreaInfo)
{
    SeedBasicAreas();
    ASSERT_TRUE(ModifySupportedArea(*mCluster, MakeNamedArea(0, "renamed bedroom")));

    DataModel::DecodableList<Structs::AreaStruct::DecodableType> areas;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::SupportedAreas::Id, areas).IsSuccess());

    auto it = areas.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_TRUE(it.GetValue().areaInfo.locationInfo.Value().locationName.data_equal("renamed bedroom"_span));
}

TEST_F(ServiceAreaClusterTest, RemovingSupportedAreaRemovesOrphanedProgress)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    ASSERT_TRUE(mCluster->AddPendingProgressElement(1));
    ASSERT_TRUE(mCluster->RemoveSupportedArea(1));

    auto progressForArea0 = FindProgressForArea(*mTester, 0);
    auto progressForArea1 = FindProgressForArea(*mTester, 1);
    ASSERT_TRUE(progressForArea0.has_value());
    EXPECT_FALSE(progressForArea1.has_value());
}

//*****************************************************************************
// SupportedMaps attribute

TEST_F(ServiceAreaClusterTest, SupportedMapsIsUnsupportedWithoutMapsFeature)
{
    DataModel::DecodableList<Structs::MapStruct::DecodableType> maps;
    EXPECT_EQ(mTester->ReadAttribute(Attributes::SupportedMaps::Id, maps).GetStatusCode(),
              ClusterStatusCode(Status::UnsupportedAttribute));
}

TEST_F(ServiceAreaClusterTest, DuplicateMapIdsOrNamesAreNotAdded)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "Main Floor"_span));
    mCluster->AddSupportedMap(1, "Duplicate Id"_span);
    mCluster->AddSupportedMap(2, "Main Floor"_span);
    mCluster->AddSupportedMap(3, ""_span);

    EXPECT_EQ(CountSupportedMaps(*mTester), 1u);
}

TEST_F(ServiceAreaClusterTest, RemovingMapRemovesItsAreas)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "floor one"_span));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(0, 1, "kitchen")));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(1, 1, "bedroom")));
    ASSERT_TRUE(mCluster->RemoveSupportedMap(1));

    EXPECT_EQ(CountSupportedMaps(*mTester), 0u);
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

// Regression test for the RemoveSupportedMap packed-buffer index bug.
//
// RemoveSupportedMap collects the areaIDs whose mapID matches the removed map
// into a temporary stack buffer, then removes each. The write into that buffer
// must use the packed cursor (count of matches so far), not the source cursor
// (index over all supported areas). When more than one map exists and the areas
// are interleaved, writing at the source cursor scatters the matches to their
// source slots; the subsequent reduce_size(<match count>) then keeps only the
// front of the buffer, so any match whose source index is >= the match count is
// dropped (never removed, and its MapID keeps pointing at the deleted map), and
// the unwritten gaps feed uninitialized stack values into RemoveSupportedAreaRaw.
//
// Two maps are needed so that removing one leaves GetNumberOfSupportedMaps() > 0
// and the buggy loop runs (the single-map case above short-circuits through
// ClearSupportedAreas). The list leads with a map-2 area so the first map-1 match
// lands past index 0: map-1 areas 100/101/102 sit at source indices 1, 3, 4. With
// 3 matches, the matches at source index >= 3 (areas 101 and 102) were dropped by
// the old code. After the fix all three map-1 areas are removed and the two map-2
// areas are kept.
TEST_F(ServiceAreaClusterTest, RemoveSupportedMapRemovesAllMatchingAreasWhenSparse)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "floor one"_span));
    ASSERT_TRUE(mCluster->AddSupportedMap(2, "floor two"_span));

    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(200, 2, "two hundred"))); // source index 0: non-match
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(100, 1, "one hundred"))); // source index 1: match
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(201, 2, "two oh one")));  // source index 2: non-match
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(101, 1, "one oh one")));  // source index 3: match, dropped by bug
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(102, 1, "one oh two")));  // source index 4: match, dropped by bug

    ASSERT_TRUE(mCluster->RemoveSupportedMap(1));

    std::vector<uint32_t> remaining = ReadSupportedAreaIds(*mTester);

    // Every area that referenced the removed map must be gone; 101 and 102 are the
    // ones the old code left behind.
    EXPECT_FALSE(Contains(remaining, 100u));
    EXPECT_FALSE(Contains(remaining, 101u));
    EXPECT_FALSE(Contains(remaining, 102u));

    // Areas belonging to the surviving map must be untouched.
    EXPECT_TRUE(Contains(remaining, 200u));
    EXPECT_TRUE(Contains(remaining, 201u));

    EXPECT_EQ(remaining.size(), 2u);
    EXPECT_EQ(CountSupportedMaps(*mTester), 1u);
}

TEST_F(ServiceAreaClusterTest, ClearingSupportedMapsClearsSupportedAreas)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "floor one"_span));
    ASSERT_TRUE(AddSupportedArea(*mCluster, MakeMappedArea(0, 1, "kitchen")));
    ASSERT_TRUE(mCluster->ClearSupportedMaps());

    EXPECT_EQ(CountSupportedMaps(*mTester), 0u);
    EXPECT_EQ(CountSupportedAreas(*mTester), 0u);
}

TEST_F(ServiceAreaClusterTest, RenamedMapIsVisibleOnRead)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "old name"_span));
    ASSERT_TRUE(mCluster->RenameSupportedMap(1, "new name"_span));

    DataModel::DecodableList<Structs::MapStruct::DecodableType> maps;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::SupportedMaps::Id, maps).IsSuccess());

    auto it = maps.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_TRUE(it.GetValue().name.data_equal("new name"_span));
}

TEST_F(ServiceAreaClusterTest, SupportedMapsListIsUnchangedWhenDelegateDisallowsClear)
{
    CreateCluster({ .features = BitMask<Feature>(Feature::kMaps), .optionalAttributes = AllOptionalAttributes() });

    ASSERT_TRUE(mCluster->AddSupportedMap(1, "floor"_span));
    mDelegate.mSupportedMapChangeAllowed = false;
    mCluster->ClearSupportedMaps();

    EXPECT_EQ(CountSupportedMaps(*mTester), 1u);
}

//*****************************************************************************
// SelectedAreas attribute

TEST_F(ServiceAreaClusterTest, SelectedAreasStartsEmpty)
{
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

TEST_F(ServiceAreaClusterTest, SelectedAreasOnlyContainsSupportedAreaIds)
{
    AddSelectedArea(*mCluster, static_cast<uint32_t>(0));
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());

    SeedBasicAreas();
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(0)));

    auto selected = ReadSelectedAreasList(*mTester);
    ASSERT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected[0], 0u);
}

TEST_F(ServiceAreaClusterTest, SelectedAreasDoesNotContainDuplicates)
{
    SeedBasicAreas();
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(0)));
    AddSelectedArea(*mCluster, static_cast<uint32_t>(0));

    auto selected = ReadSelectedAreasList(*mTester);
    ASSERT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected[0], 0u);
}

TEST_F(ServiceAreaClusterTest, SelectedAreasIsUnchangedWhenDelegateDisallowsSelection)
{
    SeedBasicAreas();
    mDelegate.mSetSelectedAreasAllowed = false;
    AddSelectedArea(*mCluster, static_cast<uint32_t>(0));
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

//*****************************************************************************
// CurrentArea and EstimatedEndTime attributes

TEST_F(ServiceAreaClusterTest, CurrentAreaStartsNull)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    DataModel::Nullable<uint32_t> currentArea;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::CurrentArea::Id, currentArea).IsSuccess());
    EXPECT_TRUE(currentArea.IsNull());
}

TEST_F(ServiceAreaClusterTest, CurrentAreaOnlyReflectsSupportedAreas)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(99)));

    DataModel::Nullable<uint32_t> currentArea;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::CurrentArea::Id, currentArea).IsSuccess());
    EXPECT_TRUE(currentArea.IsNull());

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));

    EXPECT_TRUE(mTester->ReadAttribute(Attributes::CurrentArea::Id, currentArea).IsSuccess());
    EXPECT_FALSE(currentArea.IsNull());
    EXPECT_EQ(currentArea.Value(), 0u);
}

TEST_F(ServiceAreaClusterTest, ClearingCurrentAreaClearsEstimatedEndTime)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(1000))));
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::NullNullable));

    DataModel::Nullable<uint32_t> estimatedEndTime;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::EstimatedEndTime::Id, estimatedEndTime).IsSuccess());
    EXPECT_TRUE(estimatedEndTime.IsNull());
}

TEST_F(ServiceAreaClusterTest, EstimatedEndTimeRemainsNullWhileCurrentAreaIsNull)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(100)));

    DataModel::Nullable<uint32_t> estimatedEndTime;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::EstimatedEndTime::Id, estimatedEndTime).IsSuccess());
    EXPECT_TRUE(estimatedEndTime.IsNull());
}

TEST_F(ServiceAreaClusterTest, EstimatedEndTimeChangeToNullIsReported)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(500))));
    mTester->GetDirtyList().clear();

    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::NullNullable));
    EXPECT_TRUE(DirtyListContainsAttribute(*mTester, Attributes::EstimatedEndTime::Id));
}

TEST_F(ServiceAreaClusterTest, EstimatedEndTimeChangeToZeroIsReported)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(500))));
    mTester->GetDirtyList().clear();

    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    EXPECT_TRUE(DirtyListContainsAttribute(*mTester, Attributes::EstimatedEndTime::Id));
}

TEST_F(ServiceAreaClusterTest, EstimatedEndTimeDecreaseIsReported)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(500))));
    mTester->GetDirtyList().clear();

    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(400))));
    EXPECT_TRUE(DirtyListContainsAttribute(*mTester, Attributes::EstimatedEndTime::Id));
}

TEST_F(ServiceAreaClusterTest, EstimatedEndTimeIncreaseIsNotReported)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::CurrentArea::Id>();
    optionalAttributes.Set<ServiceArea::Attributes::EstimatedEndTime::Id>();
    CreateCluster({ .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->SetCurrentArea(DataModel::MakeNullable(static_cast<uint32_t>(0))));
    ASSERT_TRUE(mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(500))));
    mTester->GetDirtyList().clear();

    mCluster->SetEstimatedEndTime(DataModel::MakeNullable(static_cast<uint32_t>(600)));
    EXPECT_FALSE(DirtyListContainsAttribute(*mTester, Attributes::EstimatedEndTime::Id));
}

//*****************************************************************************
// Progress attribute

TEST_F(ServiceAreaClusterTest, ProgressIsUnsupportedWithoutOptionalAttribute)
{
    DataModel::DecodableList<Structs::ProgressStruct::DecodableType> progress;
    EXPECT_EQ(mTester->ReadAttribute(Attributes::Progress::Id, progress).GetStatusCode(),
              ClusterStatusCode(Status::UnsupportedAttribute));
}

TEST_F(ServiceAreaClusterTest, NewProgressStartsPendingWithNullTotalOperationalTime)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));

    auto progress = FindProgressForArea(*mTester, 0);
    EXPECT_EQ(ProgressElementStatus(progress), OperationalStatusEnum::kPending);
    EXPECT_EQ(ProgressElementHasTotalOperationalTime(progress), false);
}

TEST_F(ServiceAreaClusterTest, ProgressForUnsupportedAreaDoesNotAppear)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    mCluster->AddPendingProgressElement(0);
    EXPECT_FALSE(FindProgressForArea(*mTester, 0).has_value());
}

TEST_F(ServiceAreaClusterTest, LeavingCompletedOrOperatingClearsTotalOperationalTime)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    ASSERT_TRUE(mCluster->SetProgressStatus(0, OperationalStatusEnum::kCompleted));
    ASSERT_TRUE(mCluster->SetProgressTotalOperationalTime(0, DataModel::MakeNullable(static_cast<uint32_t>(120))));
    ASSERT_TRUE(mCluster->SetProgressStatus(0, OperationalStatusEnum::kOperating));

    auto progress = FindProgressForArea(*mTester, 0);
    EXPECT_EQ(ProgressElementHasTotalOperationalTime(progress), false);
}

TEST_F(ServiceAreaClusterTest, TotalOperationalTimeIsStoredForCompletedAreas)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    ASSERT_TRUE(mCluster->SetProgressStatus(0, OperationalStatusEnum::kCompleted));
    ASSERT_TRUE(mCluster->SetProgressTotalOperationalTime(0, DataModel::MakeNullable(static_cast<uint32_t>(300))));

    auto progress = FindProgressForArea(*mTester, 0);
    EXPECT_EQ(ProgressElementHasTotalOperationalTime(progress), true);
    EXPECT_EQ(ProgressElementTotalOperationalTimeSeconds(progress), 300u);
}

TEST_F(ServiceAreaClusterTest, TotalOperationalTimeIsNotStoredWhileAreaIsPending)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    mCluster->SetProgressTotalOperationalTime(0, DataModel::MakeNullable(static_cast<uint32_t>(60)));

    auto progress = FindProgressForArea(*mTester, 0);
    EXPECT_EQ(ProgressElementHasTotalOperationalTime(progress), false);
}

TEST_F(ServiceAreaClusterTest, ProgressContainsAtMostOneEntryPerArea)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    mCluster->AddPendingProgressElement(0);

    DataModel::DecodableList<Structs::ProgressStruct::DecodableType> progressList;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::Progress::Id, progressList).IsSuccess());

    size_t count = 0;
    auto it      = progressList.begin();
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(count, 1u);
}

TEST_F(ServiceAreaClusterTest, ClearingProgressEmptiesProgressAttribute)
{
    ServiceAreaCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<ServiceArea::Attributes::Progress::Id>();
    CreateCluster({ .features = BitMask<Feature>(Feature::kProgressReporting), .optionalAttributes = optionalAttributes });

    SeedBasicAreas();
    ASSERT_TRUE(mCluster->AddPendingProgressElement(0));
    ASSERT_TRUE(mCluster->ClearProgress());

    DataModel::DecodableList<Structs::ProgressStruct::DecodableType> progress;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::Progress::Id, progress).IsSuccess());
    EXPECT_FALSE(progress.begin().Next());
}

//*****************************************************************************
// SelectAreas command

TEST_F(ServiceAreaClusterTest, SelectAreasUpdatesSelectedAreasAttribute)
{
    SeedBasicAreas();
    auto result = mTester->Invoke(MakeSelectAreasRequest({ 0, 1 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kSuccess);

    auto selected = ReadSelectedAreasList(*mTester);
    ASSERT_EQ(selected.size(), 2u);
    EXPECT_EQ(selected[0], 0u);
    EXPECT_EQ(selected[1], 1u);
}

TEST_F(ServiceAreaClusterTest, SelectAreasWithEmptyListRemovesAreaConstraint)
{
    SeedBasicAreas();
    ASSERT_TRUE(AddSelectedArea(*mCluster, static_cast<uint32_t>(0)));

    auto result = mTester->Invoke(MakeSelectAreasRequest({}));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kSuccess);
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

TEST_F(ServiceAreaClusterTest, SelectAreasWithUnknownAreaLeavesSelectedAreasUnchanged)
{
    SeedBasicAreas();
    auto result = mTester->Invoke(MakeSelectAreasRequest({ 99 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kUnsupportedArea);
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

TEST_F(ServiceAreaClusterTest, SelectAreasWhileDeviceModeDisallowsSelectionLeavesSelectedAreasUnchanged)
{
    SeedBasicAreas();
    mDelegate.mSetSelectedAreasAllowed = false;
    auto result                        = mTester->Invoke(MakeSelectAreasRequest({ 0 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kInvalidInMode);
    EXPECT_TRUE(InvokeResponseHasNonEmptyStatusText(result));
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

TEST_F(ServiceAreaClusterTest, SelectAreasWithInvalidSetLeavesSelectedAreasUnchanged)
{
    SeedBasicAreas();
    mDelegate.mSelectAreasSetStatus = SelectAreasStatus::kInvalidSet;
    auto result                     = mTester->Invoke(MakeSelectAreasRequest({ 0, 1 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kInvalidSet);
    EXPECT_TRUE(ReadSelectedAreasList(*mTester).empty());
}

TEST_F(ServiceAreaClusterTest, SelectAreasIgnoresDuplicateEntriesInNewAreas)
{
    SeedBasicAreas();
    auto result = mTester->Invoke(MakeSelectAreasRequest({ 1, 0, 0, 1 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kSuccess);

    auto selected = ReadSelectedAreasList(*mTester);
    ASSERT_EQ(selected.size(), 2u);
    EXPECT_EQ(selected[0], 1u);
    EXPECT_EQ(selected[1], 0u);
}

TEST_F(ServiceAreaClusterTest, SelectAreasMatchingCurrentSelectionSucceedsWithoutChangingSelectedAreas)
{
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0, 1 })).IsSuccess());

    auto before = ReadSelectedAreasList(*mTester);
    auto result = mTester->Invoke(MakeSelectAreasRequest({ 0, 1 }));
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SelectAreasResponseStatus(result), SelectAreasStatus::kSuccess);
    EXPECT_EQ(ReadSelectedAreasList(*mTester), before);
}

//*****************************************************************************
// SkipArea command

TEST_F(ServiceAreaClusterTest, SkipAreaIsUnsupportedWithoutCurrentAreaOrProgress)
{
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0 })).IsSuccess());

    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedCommand));
}

TEST_F(ServiceAreaClusterTest, SkipAreaWithoutSelectedAreasReturnsInvalidAreaList)
{
    CreateCluster(ClusterConfigWithCurrentArea());
    SeedBasicAreas();
    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidAreaList);
}

TEST_F(ServiceAreaClusterTest, SkipAreaWithUnknownAreaReturnsInvalidSkippedArea)
{
    CreateCluster(ClusterConfigWithCurrentArea());
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0 })).IsSuccess());

    Commands::SkipArea::Type request{ .skippedArea = 99 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidSkippedArea);
}

TEST_F(ServiceAreaClusterTest, SkipAreaWhileDeviceModeDisallowsSkipReturnsInvalidInMode)
{
    CreateCluster(ClusterConfigWithCurrentArea());
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0 })).IsSuccess());
    mDelegate.mSkipAreaAllowed = false;

    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidInMode);
    EXPECT_TRUE(InvokeResponseHasNonEmptyStatusText(result));
}

TEST_F(ServiceAreaClusterTest, SkipAreaSuccessInvokesDeviceDelegate)
{
    CreateCluster(ClusterConfigWithCurrentArea());
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0, 1 })).IsSuccess());

    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kSuccess);
    EXPECT_EQ(mDelegate.mLastSkippedArea, 0u);
}

TEST_F(ServiceAreaClusterTest, SkipAreaIsRejectedAfterUnconstrainedSelection)
{
    CreateCluster(ClusterConfigWithCurrentArea());
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({})).IsSuccess());

    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidAreaList);
}

} // namespace Testing
} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
