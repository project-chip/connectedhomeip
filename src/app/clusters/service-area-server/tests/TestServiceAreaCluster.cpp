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

#include "TestServiceAreaClusterCommon.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ServiceArea/Attributes.h>
#include <clusters/ServiceArea/Metadata.h>
#include <clusters/shared/Attributes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {
namespace Testing {

using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::ServiceArea;
using namespace chip::Protocols::InteractionModel;

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

TEST_F(ServiceAreaClusterTest, SelectAreasAndSkipAreaCommandsAreAccepted)
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

TEST_F(ServiceAreaClusterTest, SkipAreaWithoutSelectedAreasReturnsInvalidAreaList)
{
    SeedBasicAreas();
    Commands::SkipArea::Type request{ .skippedArea = 0 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidAreaList);
}

TEST_F(ServiceAreaClusterTest, SkipAreaWithUnknownAreaReturnsInvalidSkippedArea)
{
    SeedBasicAreas();
    ASSERT_TRUE(mTester->Invoke(MakeSelectAreasRequest({ 0 })).IsSuccess());

    Commands::SkipArea::Type request{ .skippedArea = 99 };
    auto result = mTester->Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(SkipAreaResponseStatus(result), SkipAreaStatus::kInvalidSkippedArea);
}

TEST_F(ServiceAreaClusterTest, SkipAreaWhileDeviceModeDisallowsSkipReturnsInvalidInMode)
{
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
