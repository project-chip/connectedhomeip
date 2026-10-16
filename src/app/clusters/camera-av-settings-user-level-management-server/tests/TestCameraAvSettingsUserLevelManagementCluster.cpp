/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementConstants.h>
#include <app/clusters/camera-av-settings-user-level-management-server/MigrateCameraAvSettingsUserLevelManagementCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Attributes.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Commands.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Enums.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Defer.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/ScopedMemoryBuffer.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using namespace Protocols::InteractionModel;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

static constexpr chip::EndpointId kTestEndpointId = 1;
static constexpr uint8_t testMaxPresets           = 5;

class TestableCameraAvSettingsUserLevelManagementLogic : public CameraAvSettingsUserLevelMgmtServerLogic
{
public:
    using CameraAvSettingsUserLevelMgmtServerLogic::CameraAvSettingsUserLevelMgmtServerLogic;
    using CameraAvSettingsUserLevelMgmtServerLogic::UpdatePresetID;
};

class MockCameraAvSettingsUserLevelManagementDelegate : public CameraAvSettingsUserLevelManagementDelegate
{
public:
    void ShutdownApp() override {}
    bool CanChangeMPTZ() override { return true; }

    void VideoStreamAllocated(uint16_t aStreamID) override {}
    void VideoStreamDeallocated(uint16_t aStreamID) override {}
    void DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport) override {}
    Status MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                           PhysicalPTZCallback * callback) override
    {
        return Status::Success;
    }

    Status MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                            PhysicalPTZCallback * callback) override
    {
        return Status::Success;
    }

    Status MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                            PhysicalPTZCallback * callback) override
    {
        return Status::Success;
    }

    Status MPTZSavePreset(uint8_t aPreset) override
    {
        ++mSavePresetCalls;
        mLastSavedPresetID = aPreset;
        return mSavePresetResult;
    }

    Status MPTZRemovePreset(uint8_t aPreset) override
    {
        ++mRemovePresetCalls;
        return mRemovePresetResult;
    }

    size_t mSavePresetCalls                           = 0;
    uint8_t mLastSavedPresetID                        = 0;
    Status mSavePresetResult                          = Status::Success;
    size_t mRemovePresetCalls                         = 0;
    Status mRemovePresetResult                        = Status::Success;
    CameraAvSettingsUserLevelMgmtServerLogic * mLogic = nullptr;
    uint8_t mPresetIDAtLoad                           = 0;

    Status DPTZSetViewport(uint16_t aVideoStreamID, Globals::Structs::ViewportStruct::Type aViewport) override
    {
        return Status::Success;
    }

    Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX, Optional<int16_t> aDeltaY,
                            Optional<int8_t> aZoomDelta, Globals::Structs::ViewportStruct::Type & aViewport) override
    {
        return Status::Success;
    }

    CHIP_ERROR PersistentAttributesLoadedCallback() override
    {
        if (mLogic != nullptr)
        {
            mPresetIDAtLoad = mLogic->mCurrentPresetID;
        }
        return CHIP_NO_ERROR;
    }
};

struct TestCameraAvSettingsUserLevelManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestCameraAvSettingsUserLevelManagementCluster() :
        mServer(kTestEndpointId,
                chip::BitFlags<Feature>(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                        Feature::kMechanicalZoom, Feature::kMechanicalPresets),
                testMaxPresets),
        mClusterTester(mServer)
    {}

    void SetUp() override
    {
        mServer.SetDelegate(&mMockDelegate);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override { mServer.Shutdown(ClusterShutdownType::kClusterShutdown); }

    MockCameraAvSettingsUserLevelManagementDelegate mMockDelegate;
    CameraAvSettingsUserLevelManagementCluster mServer;
    ClusterTester mClusterTester;
};

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, TestAttributes)
{
    ASSERT_TRUE(IsAttributesListEqualTo(mServer,
                                        {
                                            Attributes::MPTZPosition::kMetadataEntry,
                                            Attributes::MaxPresets::kMetadataEntry,
                                            Attributes::MPTZPresets::kMetadataEntry,
                                            Attributes::DPTZStreams::kMetadataEntry,
                                            Attributes::ZoomMax::kMetadataEntry,
                                            Attributes::TiltMin::kMetadataEntry,
                                            Attributes::TiltMax::kMetadataEntry,
                                            Attributes::PanMin::kMetadataEntry,
                                            Attributes::PanMax::kMetadataEntry,
                                            Attributes::MovementState::kMetadataEntry,
                                        }));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, TestCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mServer,
                                              {
                                                  Commands::MPTZSetPosition::kMetadataEntry,
                                                  Commands::MPTZRelativeMove::kMetadataEntry,
                                                  Commands::MPTZMoveToPreset::kMetadataEntry,
                                                  Commands::MPTZSavePreset::kMetadataEntry,
                                                  Commands::MPTZRemovePreset::kMetadataEntry,
                                                  Commands::DPTZSetViewport::kMetadataEntry,
                                                  Commands::DPTZRelativeMove::kMetadataEntry,
                                              }));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReadAllAttributesWithClusterTesterTest)
{
    uint8_t maxPresets = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MaxPresets::Id, maxPresets), CHIP_NO_ERROR);
    ASSERT_EQ(testMaxPresets, maxPresets);

    uint8_t zoomMax = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ZoomMax::Id, zoomMax), CHIP_NO_ERROR);
    ASSERT_LE(zoomMax, kZoomMaxMaxValue);
    ASSERT_GE(zoomMax, kZoomMaxMinValue);

    int16_t tiltMin = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TiltMin::Id, tiltMin), CHIP_NO_ERROR);
    ASSERT_LE(tiltMin, kTiltMinMaxValue);
    ASSERT_GE(tiltMin, kTiltMinMinValue);

    int16_t tiltMax = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TiltMax::Id, tiltMax), CHIP_NO_ERROR);
    ASSERT_LE(tiltMax, kTiltMaxMaxValue);
    ASSERT_GE(tiltMax, kTiltMaxMinValue);

    int16_t panMin = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::PanMin::Id, panMin), CHIP_NO_ERROR);
    ASSERT_LE(panMin, kPanMinMaxValue);
    ASSERT_GE(panMin, kPanMinMinValue);

    int16_t panMax = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::PanMax::Id, panMax), CHIP_NO_ERROR);
    ASSERT_LE(panMax, kPanMaxMaxValue);
    ASSERT_GE(panMax, kPanMaxMinValue);

    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_LE(mptzPosition.pan.Value(), panMax);
    ASSERT_GE(mptzPosition.pan.Value(), panMin);
    ASSERT_LE(mptzPosition.tilt.Value(), tiltMax);
    ASSERT_GE(mptzPosition.tilt.Value(), tiltMin);
    ASSERT_LE(mptzPosition.zoom.Value(), zoomMax);
    ASSERT_GE(mptzPosition.zoom.Value(), kZoomMinValue);

    // No commands have been invoked, we should be Idle
    PhysicalMovementEnum movementState;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);

    // On startup there should be no presets and no streams set
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZSetPositionCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSetPosition::Id };
    Commands::MPTZSetPosition::DecodableType commandData;

    // Set new values of pan, tilt, zoom to be the mid-point of their allowed ranges
    int16_t testPan  = (kPanMaxMaxValue - kPanMinMinValue) / 2;
    int16_t testTilt = (kTiltMaxMaxValue - kTiltMinMinValue) / 2;
    uint8_t testZoom = (kZoomMaxMaxValue - kZoomMinValue) / 2;

    commandData.pan.Emplace(testPan);
    commandData.tilt.Emplace(testTilt);
    commandData.zoom.Emplace(testZoom);

    auto response = mServer.GetLogic().HandleMPTZSetPosition(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus. We're explicitly checking as clang complains on use
    // of Optional without checking if a form ASSERT_TRUE(response.has_value()) is used.
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Server should think the device is moving until the app callsback, verify that the MovementState is correct
    PhysicalMovementEnum movementState;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the delegate callback
    mServer.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);

    // Verify that the MovementState has returned to Idle
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZRelativeMoveCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZRelativeMove::Id };
    Commands::MPTZRelativeMove::DecodableType commandData;

    // Get the current values of MPTZ, these should be the server defaults (0,0,1)
    Structs::MPTZStruct::DecodableType startingMptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, startingMptzPosition), CHIP_NO_ERROR);

    // Set relative values of pan, tilt, zoom to be the +10
    int16_t relativePan  = 10;
    int16_t relativeTilt = 10;
    int8_t relativeZoom  = 10;

    commandData.panDelta.Emplace(relativePan);
    commandData.tiltDelta.Emplace(relativeTilt);
    commandData.zoomDelta.Emplace(relativeZoom);

    auto response = mServer.GetLogic().HandleMPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Server should think the device is moving until the app callsback, verify that the MovementState is correct
    PhysicalMovementEnum movementState;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the delegate callback
    mServer.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType movedMptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, movedMptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(movedMptzPosition.pan.Value(), startingMptzPosition.pan.Value() + relativePan);
    ASSERT_EQ(movedMptzPosition.tilt.Value(), startingMptzPosition.tilt.Value() + relativeTilt);
    ASSERT_EQ(movedMptzPosition.zoom.Value(), startingMptzPosition.zoom.Value() + relativeZoom);

    // Verify that the MovementState has returned to Idle
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);
}

/**
 * @brief Verify an explicit save captures the current position and retains the already-free allocation cursor.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZSavePresetCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType commandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Get the current values of MPTZ, these should be the server defaults (0,0,1)
    Structs::MPTZStruct::DecodableType currentMptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, currentMptzPosition), CHIP_NO_ERROR);

    // Save the current values as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    uint8_t presetIDAsInt = 2;

    commandData.name = presetName;
    commandData.presetID.Emplace(presetIDAsInt);
    auto response = mServer.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Verify that there is a single saved preset. The values match those of the current MPTZ Position
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    auto it = mptzPresets.begin();
    while (it.Next())
    {
        ASSERT_EQ(*it.GetValue().name.data(), *presetName.data());
        ASSERT_EQ(it.GetValue().presetID, presetIDAsInt);
    }

    // As we provided a Preset ID of 2, the next Preset to use should still be 1
    ASSERT_EQ(mServer.GetLogic().mCurrentPresetID, 1);
}

/**
 * @brief Rebuild the cursor from sparse persisted presets before the loaded-attributes callback and an implicit save.
 * Presets 1 and 3 must remain unchanged while the new command uses free ID 2.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReloadedPresetsAllocateUnusedID)
{
    const uint8_t savedIDs[] = { 1, 3 };
    for (uint8_t id : savedIDs)
    {
        mServer.GetLogic().SetPan(MakeOptional(static_cast<int16_t>(id)));
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = id == 1 ? "SavedOne"_span : "SavedThree"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
    restoredDelegate.mLogic = &restored.GetLogic();
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 2);
    EXPECT_EQ(restoredDelegate.mPresetIDAtLoad, 2);
    ClusterTester restoredTester(restored);
    Commands::MPTZSavePreset::Type save;
    save.name = "AddedAfterReload"_span;
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, 2);
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 4);
    Attributes::MPTZPresets::TypeInfo::DecodableType presets;
    ASSERT_EQ(restoredTester.ReadAttribute(Attributes::MPTZPresets::Id, presets), CHIP_NO_ERROR);
    const uint8_t expectedIDs[] = { 1, 3, 2 };
    const CharSpan names[]      = { "SavedOne"_span, "SavedThree"_span, "AddedAfterReload"_span };
    auto it                     = presets.begin();
    size_t index                = 0;
    for (uint8_t id : expectedIDs)
    {
        ASSERT_TRUE(it.Next());
        EXPECT_EQ(it.GetValue().presetID, id);
        EXPECT_TRUE(it.GetValue().name.data_equal(names[index]));
        ASSERT_TRUE(it.GetValue().settings.pan.HasValue());
        EXPECT_EQ(it.GetValue().settings.pan.Value(), static_cast<int16_t>(id == 1 ? 1 : 3));
        ++index;
    }
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
}

/** @brief A sparse reload with ID 1 already free must preserve that ID instead of skipping to ID 2. */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReloadKeepsFirstUnusedID)
{
    Commands::MPTZSavePreset::Type save;
    save.presetID.SetValue(3);
    save.name = "SavedThree"_span;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 1);
    ClusterTester restoredTester(restored);
    save.presetID.ClearValue();
    save.name = "AddedOne"_span;
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, 1);
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 2);
    ASSERT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), 2u);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers[0].GetPresetID(), 3);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers[1].GetPresetID(), 1);
}

/**
 * @brief A reloaded full list must reject creation, then allocate the freed ID after one successful removal.
 * The final save must expose each ID exactly once and retain the full-list capacity behavior.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReloadFullListReusesRemovedID)
{
    for (uint8_t id = 1; id <= testMaxPresets; ++id)
    {
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = "SavedPreset"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ClusterTester restoredTester(restored);
    mClusterTester.GetDirtyList().clear();
    Commands::MPTZSavePreset::Type save;
    save.name = "Replacement"_span;
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(),
              ClusterStatusCode(Status::ResourceExhausted));
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 0u);
    EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
    Commands::MPTZRemovePreset::Type remove;
    remove.presetID = 3;
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZRemovePreset::Id, remove).IsSuccess());
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 3);
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, 3);
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 1u);
    Attributes::MPTZPresets::TypeInfo::DecodableType presets;
    ASSERT_EQ(restoredTester.ReadAttribute(Attributes::MPTZPresets::Id, presets), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(presets.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, static_cast<size_t>(testMaxPresets));
    bool seen[testMaxPresets + 1] = {};
    auto it                       = presets.begin();
    while (it.Next())
    {
        const uint8_t id = it.GetValue().presetID;
        ASSERT_GE(id, 1);
        ASSERT_LE(id, testMaxPresets);
        EXPECT_FALSE(seen[id]);
        seen[id] = true;
        EXPECT_TRUE(it.GetValue().name.data_equal(id == 3 ? "Replacement"_span : "SavedPreset"_span));
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    for (uint8_t id = 1; id <= testMaxPresets; ++id)
    {
        EXPECT_TRUE(seen[id]);
    }
    mClusterTester.GetDirtyList().clear();
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(),
              ClusterStatusCode(Status::ResourceExhausted));
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 1u);
    EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReloadOversizedListRejectsImplicitSave)
{
    for (uint8_t id = 1; id <= testMaxPresets; ++id)
    {
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = "P"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }

    constexpr uint8_t reducedMaxPresets = testMaxPresets - 1;
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, reducedMaxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ASSERT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), static_cast<size_t>(testMaxPresets));

    ClusterTester restoredTester(restored);
    Commands::MPTZSavePreset::Type save;
    save.name = "Rejected"_span;
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(),
              ClusterStatusCode(Status::ResourceExhausted));
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 0u);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), static_cast<size_t>(testMaxPresets));
}

/**
 * @brief Restarting the same object must rebuild a cursor made stale by valid saves while it was stopped.
 * Another cluster instance adds ID 4 to storage; the restarted cluster must allocate ID 5.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, RestartRebuildsStalePresetID)
{
    for (uint8_t id = 1; id <= 3; ++id)
    {
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = "SavedBeforeStop"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    ASSERT_EQ(mServer.GetLogic().mCurrentPresetID, 4);
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    {
        MockCameraAvSettingsUserLevelManagementDelegate otherDelegate;
        CameraAvSettingsUserLevelManagementCluster other(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
        other.SetDelegate(&otherDelegate);
        ASSERT_EQ(other.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        auto shutdown = MakeDefer([&other] { other.Shutdown(ClusterShutdownType::kClusterShutdown); });
        ClusterTester otherTester(other);
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(4);
        save.name = "AddedWhileStopped"_span;
        ASSERT_TRUE(otherTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    mMockDelegate.mLogic = &mServer.GetLogic();
    ASSERT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(mServer.GetLogic().mCurrentPresetID, 5);
    EXPECT_EQ(mMockDelegate.mPresetIDAtLoad, 5);
    Commands::MPTZSavePreset::Type save;
    save.name = "AddedAfterRestart"_span;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(mMockDelegate.mLastSavedPresetID, 5);
    ASSERT_EQ(mServer.GetLogic().mMptzPresetHelpers.size(), 5u);
    for (size_t index = 0; index < testMaxPresets; ++index)
    {
        EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[index].GetPresetID(), static_cast<uint8_t>(index + 1));
    }
}

/**
 * @brief Reload must find free ID 255 when all lower IDs were saved through valid commands.
 * Decode persistence directly to check all IDs without depending on a single-report attribute buffer size.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ReloadAllocatesLastAvailableID)
{
    constexpr uint8_t maxPresets = 255;
    {
        MockCameraAvSettingsUserLevelManagementDelegate initialDelegate;
        CameraAvSettingsUserLevelManagementCluster initial(kTestEndpointId, mServer.GetLogic().mFeatures, maxPresets);
        initial.SetDelegate(&initialDelegate);
        ASSERT_EQ(initial.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        auto shutdown = MakeDefer([&initial] { initial.Shutdown(ClusterShutdownType::kClusterShutdown); });
        ClusterTester initialTester(initial);
        for (uint16_t id = 1; id < maxPresets; ++id)
        {
            Commands::MPTZSavePreset::Type save;
            save.presetID.SetValue(static_cast<uint8_t>(id));
            save.name = "P"_span;
            ASSERT_TRUE(initialTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
        }
    }
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, maxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, maxPresets);
    ClusterTester restoredTester(restored);
    Commands::MPTZSavePreset::Type save;
    save.name = "LastPreset"_span;
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, maxPresets);
    ASSERT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), static_cast<size_t>(maxPresets));
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers.back().GetPresetID(), maxPresets);

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    ASSERT_TRUE(buffer.Alloc(kMaxMPTZPresetsSerializedSize));
    MutableByteSpan bytes(buffer.Get(), kMaxMPTZPresetsSerializedSize);
    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    ASSERT_EQ(mClusterTester.GetTestContext().AttributePersistenceProvider().ReadValue(path, bytes), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(bytes);
    ASSERT_EQ(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()), CHIP_NO_ERROR);
    Attributes::MPTZPresets::TypeInfo::DecodableType presets;
    ASSERT_EQ(DataModel::Decode(reader, presets), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(presets.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, static_cast<size_t>(maxPresets));
    bool seen[256] = {};
    auto it        = presets.begin();
    while (it.Next())
    {
        const uint8_t id = it.GetValue().presetID;
        EXPECT_NE(id, 0);
        EXPECT_FALSE(seen[id]);
        seen[id] = true;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    for (uint16_t id = 1; id <= maxPresets; ++id)
    {
        EXPECT_TRUE(seen[id]);
    }
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(),
              ClusterStatusCode(Status::ResourceExhausted));
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 1u);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ZeroCapacityPresetIDUpdateDoesNotDivideByZero)
{
    MockCameraAvSettingsUserLevelManagementDelegate delegate;
    CameraAvSettingsUserLevelManagementCluster cluster(kTestEndpointId, BitFlags<Feature>(Feature::kMechanicalPan), 0);
    cluster.SetDelegate(&delegate);
    ASSERT_EQ(cluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&cluster] { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); });
    EXPECT_TRUE(cluster.GetLogic().mMptzPresetHelpers.empty());
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(
        cluster, { Commands::MPTZSetPosition::kMetadataEntry, Commands::MPTZRelativeMove::kMetadataEntry }));

    TestableCameraAvSettingsUserLevelManagementLogic logic(kTestEndpointId, {}, 0);
    MPTZPresetHelper preset;
    preset.SetPresetID(1);
    logic.mMptzPresetHelpers.push_back(preset);
    logic.UpdatePresetID();
    EXPECT_EQ(logic.mCurrentPresetID, 1);
}

/**
 * @brief Delegate and persistence failures after reload must preserve the available ID, presets, storage, and notifications.
 * A subsequent successful retry must use that same free ID.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, SaveFailuresPreserveReloadedAvailableID)
{
    const uint8_t ids[] = { 1, 3 };
    for (uint8_t id : ids)
    {
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = "PreservedPreset"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    auto & persistence = mClusterTester.GetTestContext().AttributePersistenceProvider();
    uint8_t beforeBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan before(beforeBuffer);
    ASSERT_EQ(persistence.ReadValue(path, before), CHIP_NO_ERROR);
    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ClusterTester restoredTester(restored);
    mClusterTester.GetDirtyList().clear();
    Commands::MPTZSavePreset::Type save;
    save.name                          = "RetryPreset"_span;
    restoredDelegate.mSavePresetResult = Status::Busy;
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(), ClusterStatusCode(Status::Busy));
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 2);
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, 2);
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 1u);
    EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
    restoredDelegate.mSavePresetResult = Status::Success;
    auto & storage                     = mClusterTester.GetTestContext().StorageDelegate();
    storage.SetRejectWrites(true);
    auto allowWrites = MakeDefer([&storage] { storage.SetRejectWrites(false); });
    EXPECT_EQ(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).GetStatusCode(), ClusterStatusCode(Status::Failure));
    EXPECT_EQ(restored.GetLogic().mCurrentPresetID, 2);
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 2u);
    EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
    ASSERT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), 2u);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers[0].GetPresetID(), 1);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers[1].GetPresetID(), 3);
    uint8_t afterBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan after(afterBuffer);
    ASSERT_EQ(persistence.ReadValue(path, after), CHIP_NO_ERROR);
    EXPECT_TRUE(before.data_equal(after));
    storage.SetRejectWrites(false);
    ASSERT_TRUE(restoredTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    EXPECT_EQ(restoredDelegate.mLastSavedPresetID, 2);
    EXPECT_EQ(restoredDelegate.mSavePresetCalls, 3u);
    EXPECT_TRUE(mClusterTester.IsAttributeDirty(Attributes::MPTZPresets::Id));
    ASSERT_EQ(restored.GetLogic().mMptzPresetHelpers.size(), 3u);
    EXPECT_EQ(restored.GetLogic().mMptzPresetHelpers.back().GetPresetID(), 2);
}

/**
 * @brief Verify a saved preset supplies its captured position after the movement completion callback.
 */
TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZMoveToPresetCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPathMove{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZMoveToPreset::Id };
    Commands::MPTZMoveToPreset::DecodableType moveToPresetCommandData;
    ConcreteCommandPath kCommandPathMPTZSet{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSetPosition::Id };
    Commands::MPTZSetPosition::DecodableType mptzSetCommandData;
    ConcreteCommandPath kCommandPathMPTZSave{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType savePresetCommandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Try to move to a preset, this should fail with NOT FOUND
    uint8_t presetIDAsInt = 2;

    moveToPresetCommandData.presetID = presetIDAsInt;
    auto moveResponse = mServer.GetLogic().HandleMPTZMoveToPreset(commandHandler, kCommandPathMove, moveToPresetCommandData);

    // The response should contain an ActionReturnStatus
    if (moveResponse.has_value())
    {
        ASSERT_EQ(moveResponse.value().GetStatusCode().GetStatus(), Status::NotFound);
    }
    else
    {
        FAIL();
    }

    // Save the current values of MPTZ as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    savePresetCommandData.name = presetName;
    savePresetCommandData.presetID.Emplace(presetIDAsInt);
    auto saveResponse = mServer.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPathMPTZSave, savePresetCommandData);

    // The response should contain an ActionReturnStatus
    if (saveResponse.has_value())
    {
        ASSERT_TRUE(saveResponse.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Save the stored preset settings; we know there is only one entry
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);

    int16_t presetPan;
    int16_t presetTilt;
    uint8_t presetZoom;

    auto it = mptzPresets.begin();
    while (it.Next())
    {
        presetPan  = it.GetValue().settings.pan.Value();
        presetTilt = it.GetValue().settings.tilt.Value();
        presetZoom = it.GetValue().settings.zoom.Value();
    }

    // Set the MPTZ position to be the mid-points of the allowed ranges
    int16_t testPan  = (kPanMaxMaxValue - kPanMinMinValue) / 2;
    int16_t testTilt = (kTiltMaxMaxValue - kTiltMinMinValue) / 2;
    uint8_t testZoom = (kZoomMaxMaxValue - kZoomMinValue) / 2;

    mptzSetCommandData.pan.Emplace(testPan);
    mptzSetCommandData.tilt.Emplace(testTilt);
    mptzSetCommandData.zoom.Emplace(testZoom);

    auto setResponse = mServer.GetLogic().HandleMPTZSetPosition(commandHandler, kCommandPathMPTZSet, mptzSetCommandData);

    // The response should contain an ActionReturnStatus
    if (setResponse.has_value())
    {
        ASSERT_TRUE(setResponse.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Fake the delegate callback
    mServer.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);

    // Move to the preset
    moveResponse = mServer.GetLogic().HandleMPTZMoveToPreset(commandHandler, kCommandPathMove, moveToPresetCommandData);

    // The response should contain an ActionReturnStatus
    if (moveResponse.has_value())
    {
        ASSERT_TRUE(moveResponse.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Ensure we're "moving"
    PhysicalMovementEnum movementState;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the callback
    mServer.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Ensure we've reverted to idle
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);

    // Ensure the values in MPTZPosition are those of the Preset
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), presetPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), presetTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), presetZoom);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZRemovePresetCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPathRemove{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZRemovePreset::Id };
    Commands::MPTZRemovePreset::DecodableType removePresetCommandData;
    ConcreteCommandPath kCommandPathMPTZSave{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType savePresetCommandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Try to remove a preset, this should fail with NOT FOUND
    uint8_t presetIDAsInt = 2;

    removePresetCommandData.presetID = presetIDAsInt;
    auto removeResponse = mServer.GetLogic().HandleMPTZRemovePreset(commandHandler, kCommandPathRemove, removePresetCommandData);

    // The response should contain an ActionReturnStatus
    if (removeResponse.has_value())
    {
        ASSERT_EQ(removeResponse.value().GetStatusCode().GetStatus(), Status::NotFound);
    }
    else
    {
        FAIL();
    }

    // Save the current values of MPTZ as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    savePresetCommandData.name = presetName;
    savePresetCommandData.presetID.Emplace(presetIDAsInt);
    auto saveResponse = mServer.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPathMPTZSave, savePresetCommandData);

    // The response should contain an ActionReturnStatus
    if (saveResponse.has_value())
    {
        ASSERT_TRUE(saveResponse.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Verify that there is a single saved preset. The values match those of the current MPTZ Position
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    // Try to remove again, verify success
    removePresetCommandData.presetID = presetIDAsInt;
    removeResponse = mServer.GetLogic().HandleMPTZRemovePreset(commandHandler, kCommandPathRemove, removePresetCommandData);

    // The response should contain an ActionReturnStatus
    if (removeResponse.has_value())
    {
        ASSERT_TRUE(removeResponse.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Verify that there are no saved presets. The values match those of the current MPTZ Position
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, RemovePresetPersistsRemainingEntries)
{
    const CharSpan names[] = { "First"_span, "Second"_span, "Third"_span, "Fourth"_span, "Fifth"_span };
    for (uint8_t id = 1; id <= testMaxPresets; ++id)
    {
        mServer.GetLogic().SetPan(MakeOptional(static_cast<int16_t>(id)));
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = names[id - 1];
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    auto & persistence               = mClusterTester.GetTestContext().AttributePersistenceProvider();
    bool removed[testMaxPresets + 1] = {};
    size_t remaining                 = testMaxPresets;

    const uint8_t removalIDs[] = { 1, 3, 5 };
    for (uint8_t id : removalIDs)
    {
        mClusterTester.GetDirtyList().clear();
        Commands::MPTZRemovePreset::Type remove;
        remove.presetID = id;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).IsSuccess());
        removed[id] = true;
        --remaining;
        EXPECT_EQ(mMockDelegate.mRemovePresetCalls, static_cast<size_t>(testMaxPresets) - remaining);
        EXPECT_TRUE(mClusterTester.IsAttributeDirty(Attributes::MPTZPresets::Id));
        EXPECT_EQ(mClusterTester.GetDirtyList().size(), 1u);

        Attributes::MPTZPresets::TypeInfo::DecodableType active;
        ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, active), CHIP_NO_ERROR);
        size_t count = 0;
        ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, remaining);

        uint8_t buffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
        MutableByteSpan bytes(buffer);
        ASSERT_EQ(persistence.ReadValue(path, bytes), CHIP_NO_ERROR);
        TLV::TLVReader reader;
        reader.Init(bytes);
        ASSERT_EQ(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()), CHIP_NO_ERROR);
        Attributes::MPTZPresets::TypeInfo::DecodableType stored;
        ASSERT_EQ(DataModel::Decode(reader, stored), CHIP_NO_ERROR);
        ASSERT_EQ(stored.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, remaining);
        auto it = stored.begin();
        for (uint8_t expected = 1; expected <= testMaxPresets; ++expected)
        {
            if (removed[expected])
            {
                continue;
            }
            ASSERT_TRUE(it.Next());
            EXPECT_EQ(it.GetValue().presetID, expected);
            EXPECT_TRUE(it.GetValue().name.data_equal(names[expected - 1]));
            ASSERT_TRUE(it.GetValue().settings.pan.HasValue());
            EXPECT_EQ(it.GetValue().settings.pan.Value(), static_cast<int16_t>(expected));
        }
        EXPECT_FALSE(it.Next());
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

        MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
        CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
        restored.SetDelegate(&restoredDelegate);
        ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
        ClusterTester restoredTester(restored);
        Attributes::MPTZPresets::TypeInfo::DecodableType reloaded;
        ASSERT_EQ(restoredTester.ReadAttribute(Attributes::MPTZPresets::Id, reloaded), CHIP_NO_ERROR);
        ASSERT_EQ(reloaded.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, remaining);
        auto reloadedIt = reloaded.begin();
        for (uint8_t expected = 1; expected <= testMaxPresets; ++expected)
        {
            if (removed[expected])
            {
                continue;
            }
            ASSERT_TRUE(reloadedIt.Next());
            EXPECT_EQ(reloadedIt.GetValue().presetID, expected);
            EXPECT_TRUE(reloadedIt.GetValue().name.data_equal(names[expected - 1]));
            ASSERT_TRUE(reloadedIt.GetValue().settings.pan.HasValue());
            EXPECT_EQ(reloadedIt.GetValue().settings.pan.Value(), static_cast<int16_t>(expected));
        }
        EXPECT_FALSE(reloadedIt.Next());
        EXPECT_EQ(reloadedIt.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(restoredDelegate.mRemovePresetCalls, 0u);
    }
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, RemoveOnlyPresetPersistsEmptyList)
{
    Commands::MPTZSavePreset::Type save;
    save.name = "01234567890123456789012345678901"_span;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    mClusterTester.GetDirtyList().clear();
    Commands::MPTZRemovePreset::Type remove;
    remove.presetID = 1;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).IsSuccess());
    EXPECT_TRUE(mServer.GetLogic().mMptzPresetHelpers.empty());
    EXPECT_EQ(mMockDelegate.mRemovePresetCalls, 1u);
    EXPECT_TRUE(mClusterTester.IsAttributeDirty(Attributes::MPTZPresets::Id));
    EXPECT_EQ(mClusterTester.GetDirtyList().size(), 1u);

    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    auto & persistence = mClusterTester.GetTestContext().AttributePersistenceProvider();
    uint8_t buffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan bytes(buffer);
    ASSERT_EQ(persistence.ReadValue(path, bytes), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(bytes);
    ASSERT_EQ(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()), CHIP_NO_ERROR);
    Attributes::MPTZPresets::TypeInfo::DecodableType stored;
    ASSERT_EQ(DataModel::Decode(reader, stored), CHIP_NO_ERROR);
    size_t storedCount = 0;
    ASSERT_EQ(stored.ComputeSize(&storedCount), CHIP_NO_ERROR);
    EXPECT_EQ(storedCount, 0u);

    MockCameraAvSettingsUserLevelManagementDelegate restoredDelegate;
    CameraAvSettingsUserLevelManagementCluster restored(kTestEndpointId, mServer.GetLogic().mFeatures, testMaxPresets);
    restored.SetDelegate(&restoredDelegate);
    ASSERT_EQ(restored.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    auto shutdown = MakeDefer([&restored] { restored.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ClusterTester restoredTester(restored);
    Attributes::MPTZPresets::TypeInfo::DecodableType presets;
    ASSERT_EQ(restoredTester.ReadAttribute(Attributes::MPTZPresets::Id, presets), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(presets.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
    EXPECT_TRUE(restored.GetLogic().mMptzPresetHelpers.empty());
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, RemovePresetStorageFailurePreservesState)
{
    for (uint8_t id = 1; id <= 3; ++id)
    {
        Commands::MPTZSavePreset::Type save;
        save.presetID.SetValue(id);
        save.name = "StoredPreset"_span;
        ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    }
    ASSERT_EQ(mServer.GetLogic().mCurrentPresetID, 4);
    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    auto & persistence = mClusterTester.GetTestContext().AttributePersistenceProvider();
    uint8_t beforeBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan before(beforeBuffer);
    ASSERT_EQ(persistence.ReadValue(path, before), CHIP_NO_ERROR);
    auto & storage = mClusterTester.GetTestContext().StorageDelegate();
    storage.SetRejectWrites(true);
    auto allowWrites = MakeDefer([&storage] { storage.SetRejectWrites(false); });
    mClusterTester.GetDirtyList().clear();

    for (uint8_t id = 1; id <= 3; ++id)
    {
        Commands::MPTZRemovePreset::Type remove;
        remove.presetID = id;
        EXPECT_EQ(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).GetStatusCode(),
                  ClusterStatusCode(Status::Failure));
        EXPECT_EQ(mMockDelegate.mRemovePresetCalls, static_cast<size_t>(id));
        EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
        EXPECT_EQ(mServer.GetLogic().mCurrentPresetID, 4);
        ASSERT_EQ(mServer.GetLogic().mMptzPresetHelpers.size(), 3u);
        for (uint8_t expected = 1; expected <= 3; ++expected)
        {
            EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[expected - 1].GetPresetID(), expected);
            EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[expected - 1].GetName(), "StoredPreset");
        }
        uint8_t afterBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
        MutableByteSpan after(afterBuffer);
        ASSERT_EQ(persistence.ReadValue(path, after), CHIP_NO_ERROR);
        EXPECT_TRUE(before.data_equal(after));
    }

    storage.SetRejectWrites(false);
    Commands::MPTZRemovePreset::Type remove;
    remove.presetID = 2;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).IsSuccess());
    EXPECT_EQ(mMockDelegate.mRemovePresetCalls, 4u);
    EXPECT_EQ(mClusterTester.GetDirtyList().size(), 1u);
    EXPECT_EQ(mServer.GetLogic().mCurrentPresetID, 4);
    ASSERT_EQ(mServer.GetLogic().mMptzPresetHelpers.size(), 2u);
    EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[0].GetPresetID(), 1);
    EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[1].GetPresetID(), 3);

    uint8_t storedBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan storedBytes(storedBuffer);
    ASSERT_EQ(persistence.ReadValue(path, storedBytes), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(storedBytes);
    ASSERT_EQ(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()), CHIP_NO_ERROR);
    Attributes::MPTZPresets::TypeInfo::DecodableType stored;
    ASSERT_EQ(DataModel::Decode(reader, stored), CHIP_NO_ERROR);
    auto it = stored.begin();
    constexpr uint8_t expectedIDs[] = { 1, 3 };
    for (uint8_t expected : expectedIDs)
    {
        ASSERT_TRUE(it.Next());
        EXPECT_EQ(it.GetValue().presetID, expected);
    }
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, RemovePresetRejectsInvalidAndDeclinedRequests)
{
    Commands::MPTZSavePreset::Type save;
    save.name = "PreservedPreset"_span;
    ASSERT_TRUE(mClusterTester.Invoke(Commands::MPTZSavePreset::Id, save).IsSuccess());
    ASSERT_EQ(mServer.GetLogic().mCurrentPresetID, 2);
    const ConcreteAttributePath path(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPresets::Id);
    auto & persistence = mClusterTester.GetTestContext().AttributePersistenceProvider();
    uint8_t beforeBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan before(beforeBuffer);
    ASSERT_EQ(persistence.ReadValue(path, before), CHIP_NO_ERROR);
    mClusterTester.GetDirtyList().clear();
    const uint8_t invalidIDs[] = { 0, 6 };
    for (uint8_t id : invalidIDs)
    {
        Commands::MPTZRemovePreset::Type remove;
        remove.presetID = id;
        EXPECT_EQ(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).GetStatusCode(),
                  ClusterStatusCode(Status::ConstraintError));
    }
    Commands::MPTZRemovePreset::Type remove;
    remove.presetID = 2;
    EXPECT_EQ(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).GetStatusCode(), ClusterStatusCode(Status::NotFound));
    EXPECT_EQ(mMockDelegate.mRemovePresetCalls, 0u);
    mMockDelegate.mRemovePresetResult = Status::Busy;
    remove.presetID                   = 1;
    EXPECT_EQ(mClusterTester.Invoke(Commands::MPTZRemovePreset::Id, remove).GetStatusCode(), ClusterStatusCode(Status::Busy));
    EXPECT_EQ(mMockDelegate.mRemovePresetCalls, 1u);
    EXPECT_TRUE(mClusterTester.GetDirtyList().empty());
    EXPECT_EQ(mServer.GetLogic().mCurrentPresetID, 2);
    ASSERT_EQ(mServer.GetLogic().mMptzPresetHelpers.size(), 1u);
    EXPECT_EQ(mServer.GetLogic().mMptzPresetHelpers[0].GetName(), "PreservedPreset");
    uint8_t afterBuffer[kMaxMPTZPresetStructSerializedSize * testMaxPresets];
    MutableByteSpan after(afterBuffer);
    ASSERT_EQ(persistence.ReadValue(path, after), CHIP_NO_ERROR);
    EXPECT_TRUE(before.data_equal(after));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteDPTZSetViewportCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::DPTZSetViewport::Id };
    Commands::DPTZSetViewport::DecodableType commandData;

    // DPTZStreams attribute starts empty
    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    // Try to set the viewport when no streams exist, this should fail with NOT FOUND
    uint8_t videoStreamID                           = 1;
    Globals::Structs::ViewportStruct::Type viewPort = { 0, 0, 1920, 1080 };
    commandData.videoStreamID                       = videoStreamID;
    commandData.viewport                            = viewPort;
    auto response = mServer.GetLogic().HandleDPTZSetViewport(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::NotFound);
    }
    else
    {
        FAIL();
    }

    // Allocate a video stream
    mServer.GetLogic().AddMoveCapableVideoStream(videoStreamID, viewPort);

    // Verify that DPTZStreams is now populated correctly
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(1));

    auto it = dptzStreams.begin();
    while (it.Next())
    {
        ASSERT_EQ(it.GetValue().videoStreamID, videoStreamID);
        ASSERT_EQ(it.GetValue().viewport.x1, viewPort.x1);
        ASSERT_EQ(it.GetValue().viewport.x2, viewPort.x2);
        ASSERT_EQ(it.GetValue().viewport.y1, viewPort.y1);
        ASSERT_EQ(it.GetValue().viewport.y2, viewPort.y2);
    }

    // Try to set the viewport
    Globals::Structs::ViewportStruct::Type newViewPort = { 0, 0, 1280, 720 };
    commandData.viewport                               = newViewPort;

    response = mServer.GetLogic().HandleDPTZSetViewport(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Verify this new viewport is in DPTZStreams
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);

    it = dptzStreams.begin();
    while (it.Next())
    {
        ASSERT_EQ(it.GetValue().videoStreamID, videoStreamID);
        ASSERT_EQ(it.GetValue().viewport.x1, newViewPort.x1);
        ASSERT_EQ(it.GetValue().viewport.x2, newViewPort.x2);
        ASSERT_EQ(it.GetValue().viewport.y1, newViewPort.y1);
        ASSERT_EQ(it.GetValue().viewport.y2, newViewPort.y2);
    }
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteDPTZRelativeMoveCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::DPTZRelativeMove::Id };
    Commands::DPTZRelativeMove::DecodableType commandData;

    // DPTZStreams attribute starts empty
    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    // Try to move the viewport when no streams exist, this should fail with NOT FOUND
    uint8_t videoStreamID = 1;
    int16_t deltaX        = 100;
    int16_t deltaY        = 100;
    int8_t zoomDelta      = -50;
    int8_t zoomDeltaFail  = 120;

    commandData.videoStreamID = videoStreamID;
    commandData.deltaX.Emplace(deltaX);
    commandData.deltaY.Emplace(deltaY);
    commandData.zoomDelta.Emplace(zoomDelta);

    auto response = mServer.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::NotFound);
    }
    else
    {
        FAIL();
    }

    // Allocate a video stream
    Globals::Structs::ViewportStruct::Type viewPort = { 0, 0, 1920, 1080 };
    mServer.GetLogic().AddMoveCapableVideoStream(videoStreamID, viewPort);

    // Try to move the viewport
    response = mServer.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // The mock delegate has no means to verify the proposed change as the values needed are part of CameraAVStreams,
    // as such, the command above should just be successful as long as the delta is in range.

    // Try to move the viewport with an out of range zoomDelta
    commandData.zoomDelta.Emplace(zoomDeltaFail);
    response = mServer.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::ConstraintError);
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZPositionPersistenceTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    // Set new values of pan, tilt, zoom to be the mid-point of their allowed ranges
    int16_t testPan  = (kPanMaxMaxValue - kPanMinMinValue) / 2;
    int16_t testTilt = (kTiltMaxMaxValue - kTiltMinMinValue) / 2;
    uint8_t testZoom = (kZoomMaxMaxValue - kZoomMinValue) / 2;

    mServer.GetLogic().SetPan(Optional<int16_t>(testPan));
    mServer.GetLogic().SetTilt(Optional<int16_t>(testTilt));
    mServer.GetLogic().SetZoom(Optional<uint8_t>(testZoom));

    // Verify that the values in the attribute are the values set
    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);

    // Shutdown the Server
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    // Start the Server back up
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Make sure the attribute value has persisted
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZPresetsPersistenceTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType commandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Get the current values of MPTZ, these should be the server defaults (0,0,1)
    Structs::MPTZStruct::DecodableType currentMptzPosition;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, currentMptzPosition), CHIP_NO_ERROR);

    // Save the current values as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    uint8_t presetIDAsInt = 2;

    commandData.name = presetName;
    commandData.presetID.Emplace(presetIDAsInt);
    auto response = mServer.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }

    // Verify that there is a single saved preset. The values match those of the current MPTZ Position
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    auto it = mptzPresets.begin();
    while (it.Next())
    {
        ASSERT_EQ(*it.GetValue().name.data(), *presetName.data());
        ASSERT_EQ(it.GetValue().presetID, presetIDAsInt);
    }

    // Shutdown the Server
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    // Start the Server back up
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read the save presets, should be a single persisted value
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    it = mptzPresets.begin();
    while (it.Next())
    {
        ASSERT_EQ(*it.GetValue().name.data(), *presetName.data());
        ASSERT_EQ(it.GetValue().presetID, presetIDAsInt);
        ASSERT_EQ(it.GetValue().settings.pan.Value(), kDefaultPan);
        ASSERT_EQ(it.GetValue().settings.tilt.Value(), kDefaultTilt);
        ASSERT_EQ(it.GetValue().settings.zoom.Value(), kDefaultZoom);
    }
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteDPTZStreamsPersistenceTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    // DPTZStreams attribute starts empty
    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    // Try to set the viewport when no streams exist, this should fail with NOT FOUND
    uint8_t videoStreamID                           = 1;
    Globals::Structs::ViewportStruct::Type viewPort = { 0, 0, 1920, 1080 };

    // Allocate a video stream
    mServer.GetLogic().AddMoveCapableVideoStream(videoStreamID, viewPort);

    // Verify that DPTZStreams is now populated correctly
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(1));

    auto it = dptzStreams.begin();
    while (it.Next())
    {
        ASSERT_EQ(it.GetValue().videoStreamID, videoStreamID);
        ASSERT_EQ(it.GetValue().viewport.x1, viewPort.x1);
        ASSERT_EQ(it.GetValue().viewport.x2, viewPort.x2);
        ASSERT_EQ(it.GetValue().viewport.y1, viewPort.y1);
        ASSERT_EQ(it.GetValue().viewport.y2, viewPort.y2);
    }

    // Shutdown the Server
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    // Start the Server back up
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Verify that DPTZStreams has the persisted entry
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(1));

    it = dptzStreams.begin();
    while (it.Next())
    {
        ASSERT_EQ(it.GetValue().videoStreamID, videoStreamID);
        ASSERT_EQ(it.GetValue().viewport.x1, viewPort.x1);
        ASSERT_EQ(it.GetValue().viewport.x2, viewPort.x2);
        ASSERT_EQ(it.GetValue().viewport.y1, viewPort.y1);
        ASSERT_EQ(it.GetValue().viewport.y2, viewPort.y2);
    }
}

// ---------------------------------------------------------------------------
// Migration tests for MigrateCameraAvSettingsUserLevelManagementCluster
// ---------------------------------------------------------------------------

struct TestMigrateCameraAvSettingsUserLevelManagementMigration : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestMigrateCameraAvSettingsUserLevelManagementMigration() :
        mServer(kTestEndpointId,
                chip::BitFlags<Feature>(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                        Feature::kMechanicalZoom, Feature::kMechanicalPresets),
                testMaxPresets),
        mClusterTester(mServer)
    {}

    void SetUp() override
    {
        mOldSafePersistence = app::GetSafeAttributePersistenceProvider();
        ASSERT_EQ(mSafePersistence.Init(&mClusterTester.GetTestContext().StorageDelegate()), CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mSafePersistence);
    }

    void TearDown() override
    {
        app::SetSafeAttributePersistenceProvider(mOldSafePersistence);
        if (mServerStarted)
        {
            mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }

    void StartServer()
    {
        mServer.SetDelegate(&mMockDelegate);
        ASSERT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        mServerStarted = true;
    }

    void WriteSafeTlv(AttributeId attrId, const ByteSpan & tlvBytes)
    {
        ASSERT_EQ(mSafePersistence.SafeWriteValue(
                      ConcreteAttributePath(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, attrId), tlvBytes),
                  CHIP_NO_ERROR);
    }

    MockCameraAvSettingsUserLevelManagementDelegate mMockDelegate;
    MigrateCameraAvSettingsUserLevelManagementCluster mServer;
    ClusterTester mClusterTester;
    app::DefaultSafeAttributePersistenceProvider mSafePersistence;
    app::SafeAttributePersistenceProvider * mOldSafePersistence = nullptr;
    bool mServerStarted                                         = false;
};

TEST_F(TestMigrateCameraAvSettingsUserLevelManagementMigration, MigratesMPTZPositionTlv)
{
    Structs::MPTZStruct::Type position;
    position.pan.SetValue(10);
    position.tilt.SetValue(20);
    position.zoom.SetValue(30);

    uint8_t buffer[CameraAvSettingsUserLevelManagement::kMptzPositionStructMaxSerializedSize];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    ASSERT_EQ(position.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
    WriteSafeTlv(Attributes::MPTZPosition::Id, ByteSpan(buffer, writer.GetLengthWritten()));
    StartServer();

    Structs::MPTZStruct::DecodableType readPosition;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPosition::Id, readPosition), CHIP_NO_ERROR);
    EXPECT_EQ(readPosition.pan.Value(), 10);
    EXPECT_EQ(readPosition.tilt.Value(), 20);
    EXPECT_EQ(readPosition.zoom.Value(), 30);
}

TEST_F(TestMigrateCameraAvSettingsUserLevelManagementMigration, MigratesMPTZPresetsTlv)
{
    Structs::MPTZStruct::Type settings;
    settings.pan.SetValue(5);
    settings.tilt.SetValue(6);
    settings.zoom.SetValue(7);

    Structs::MPTZPresetStruct::Type preset;
    preset.presetID = 2;
    preset.name     = "TestPreset"_span;
    preset.settings = settings;

    uint8_t buffer[kMaxMPTZPresetStructSerializedSize + kArrayTlvOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType arrayType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);
    ASSERT_EQ(preset.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);
    WriteSafeTlv(Attributes::MPTZPresets::Id, ByteSpan(buffer, writer.GetLengthWritten()));
    StartServer();

    Attributes::MPTZPresets::TypeInfo::DecodableType presets;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MPTZPresets::Id, presets), CHIP_NO_ERROR);
    auto it = presets.begin();
    EXPECT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().presetID, static_cast<uint8_t>(2));
    EXPECT_TRUE(it.GetValue().name.data_equal("TestPreset"_span));
    EXPECT_EQ(it.GetValue().settings.pan.Value(), 5);
    EXPECT_FALSE(it.Next());
}

TEST_F(TestMigrateCameraAvSettingsUserLevelManagementMigration, MigratesDPTZStreamsTlv)
{
    Globals::Structs::ViewportStruct::Type viewPort{ 1, 2, 100, 200 };
    Structs::DPTZStruct::Type stream;
    stream.videoStreamID = 42;
    stream.viewport      = viewPort;

    uint8_t buffer[kMaxDPTZStructSerializedSize + kArrayTlvOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType arrayType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);
    ASSERT_EQ(stream.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);
    WriteSafeTlv(Attributes::DPTZStreams::Id, ByteSpan(buffer, writer.GetLengthWritten()));
    StartServer();

    Attributes::DPTZStreams::TypeInfo::DecodableType streams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::DPTZStreams::Id, streams), CHIP_NO_ERROR);
    auto it = streams.begin();
    EXPECT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().videoStreamID, static_cast<uint16_t>(42));
    EXPECT_EQ(it.GetValue().viewport.x1, viewPort.x1);
    EXPECT_FALSE(it.Next());
}

TEST_F(TestMigrateCameraAvSettingsUserLevelManagementMigration, OldDataDeletedAfterMigration)
{
    Structs::MPTZStruct::Type position;
    position.pan.SetValue(1);
    position.tilt.SetValue(2);
    position.zoom.SetValue(3);

    uint8_t buffer[CameraAvSettingsUserLevelManagement::kMptzPositionStructMaxSerializedSize];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    ASSERT_EQ(position.Encode(writer, TLV::AnonymousTag()), CHIP_NO_ERROR);
    WriteSafeTlv(Attributes::MPTZPosition::Id, ByteSpan(buffer, writer.GetLengthWritten()));
    StartServer();

    uint8_t readBuf[CameraAvSettingsUserLevelManagement::kMptzPositionStructMaxSerializedSize];
    MutableByteSpan readSpan(readBuf);
    EXPECT_EQ(mSafePersistence.SafeReadValue(
                  ConcreteAttributePath(kTestEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPosition::Id),
                  readSpan),
              CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

} // namespace
