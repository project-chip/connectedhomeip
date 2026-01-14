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
#include <app/InteractionModelEngine.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
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
#include <lib/support/ReadOnlyBuffer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using namespace Protocols::InteractionModel;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace {

static constexpr chip::EndpointId kTestEndpointId = 1;

// Minimal mock delegate for testing
class MockCameraAvSettingsUserLevelManagementDelegate : public CameraAvSettingsUserLevelManagementDelegate
{
public:
    void ShutdownApp() {}

    bool CanChangeMPTZ() { return true; }

    void VideoStreamAllocated(uint16_t aStreamID) {}
    void VideoStreamDeallocated(uint16_t aStreamID) {}
    void DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport) {}

    Status MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom, PhysicalPTZCallback * callback)
    {
        return Status::Success;
    }

    Status MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                            PhysicalPTZCallback * callback)
    {
        return Status::Success;
    }

    Status MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                            PhysicalPTZCallback * callback)
    {
        return Status::Success;
    }

    Status MPTZSavePreset(uint8_t aPreset) { return Status::Success; }

    Status MPTZRemovePreset(uint8_t aPreset) { return Status::Success; }

    Status DPTZSetViewport(uint16_t aVideoStreamID, Globals::Structs::ViewportStruct::Type aViewport) { return Status::Success; }

    Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX, Optional<int16_t> aDeltaY,
                            Optional<int8_t> aZoomDelta, Globals::Structs::ViewportStruct::Type & aViewport)
    {
        return Status::Success;
    }

    virtual CHIP_ERROR PersistentAttributesLoadedCallback() { return CHIP_NO_ERROR; }

    CHIP_ERROR LoadMPTZPresets(std::vector<MPTZPresetHelper> & mptzPresetHelpers) { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadDPTZStreams(std::vector<Structs::DPTZStruct::Type> & dptzStreams) { return CHIP_NO_ERROR; }
};

struct TestCameraAvSettingsUserLevelManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, TestAttributes)
{
    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    ASSERT_TRUE(IsAttributesListEqualTo(server,
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
    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(server,
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
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    uint8_t maxPresets = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MaxPresets::Id, maxPresets), CHIP_NO_ERROR);
    ASSERT_EQ(testMaxPresets, maxPresets);

    uint8_t zoomMax = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::ZoomMax::Id, zoomMax), CHIP_NO_ERROR);
    ASSERT_LE(zoomMax, kZoomMaxMaxValue);
    ASSERT_GE(zoomMax, kZoomMaxMinValue);

    int16_t tiltMin = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::TiltMin::Id, tiltMin), CHIP_NO_ERROR);
    ASSERT_LE(tiltMin, kTiltMinMaxValue);
    ASSERT_GE(tiltMin, kTiltMinMinValue);

    int16_t tiltMax = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::TiltMax::Id, tiltMax), CHIP_NO_ERROR);
    ASSERT_LE(tiltMax, kTiltMaxMaxValue);
    ASSERT_GE(tiltMax, kTiltMaxMinValue);

    int16_t panMin = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::PanMin::Id, panMin), CHIP_NO_ERROR);
    ASSERT_LE(panMin, kPanMinMaxValue);
    ASSERT_GE(panMin, kPanMinMinValue);

    int16_t panMax = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::PanMax::Id, panMax), CHIP_NO_ERROR);
    ASSERT_LE(panMax, kPanMaxMaxValue);
    ASSERT_GE(panMax, kPanMaxMinValue);

    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_LE(mptzPosition.pan.Value(), panMax);
    ASSERT_GE(mptzPosition.pan.Value(), panMin);
    ASSERT_LE(mptzPosition.tilt.Value(), tiltMax);
    ASSERT_GE(mptzPosition.tilt.Value(), tiltMin);
    ASSERT_LE(mptzPosition.zoom.Value(), zoomMax);
    ASSERT_GE(mptzPosition.zoom.Value(), kZoomMinValue);

    // No commands have been invoked, we should be Idle
    PhysicalMovementEnum movementState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);

    // On startup there should be no presets and no streams set
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZSetPositionCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

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

    auto response = server.GetLogic().HandleMPTZSetPosition(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    EXPECT_TRUE(response.value().IsSuccess());

    // Server should think the device is moving until the app callsback, verify that the MovementState is correct
    PhysicalMovementEnum movementState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the delegate callback
    server.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);

    // Verify that the MovementState has returned to Idle
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZRelativeMoveCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZRelativeMove::Id };
    Commands::MPTZRelativeMove::DecodableType commandData;

    // Get the current values of MPTZ, these should be the server defaults (0,0,1)
    Structs::MPTZStruct::DecodableType startingMptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, startingMptzPosition), CHIP_NO_ERROR);

    // Set relative values of pan, tilt, zoom to be the +10
    int16_t relativePan  = 10;
    int16_t relativeTilt = 10;
    int8_t relativeZoom  = 10;

    commandData.panDelta.Emplace(relativePan);
    commandData.tiltDelta.Emplace(relativeTilt);
    commandData.zoomDelta.Emplace(relativeZoom);

    auto response = server.GetLogic().HandleMPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    EXPECT_TRUE(response.value().IsSuccess());

    // Server should think the device is moving until the app callsback, verify that the MovementState is correct
    PhysicalMovementEnum movementState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the delegate callback
    server.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType movedMptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, movedMptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(movedMptzPosition.pan.Value(), startingMptzPosition.pan.Value() + relativePan);
    ASSERT_EQ(movedMptzPosition.tilt.Value(), startingMptzPosition.tilt.Value() + relativeTilt);
    ASSERT_EQ(movedMptzPosition.zoom.Value(), startingMptzPosition.zoom.Value() + relativeZoom);

    // Verify that the MovementState has returned to Idle
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZSavePresetCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType commandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Get the current values of MPTZ, these should be the server defaults (0,0,1)
    Structs::MPTZStruct::DecodableType currentMptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, currentMptzPosition), CHIP_NO_ERROR);

    // Save the current values as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    uint8_t presetIDAsInt = 2;

    commandData.name = presetName;
    commandData.presetID.Emplace(presetIDAsInt);
    auto response = server.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    EXPECT_TRUE(response.value().IsSuccess());

    // Verify that there is a single saved preset. The values match those of the current MPTZ Position
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    auto it = mptzPresets.begin();
    while (it.Next())
    {
        ASSERT_EQ(*it.GetValue().name.data(), *presetName.data());
        ASSERT_EQ(it.GetValue().presetID, presetIDAsInt);
    }

    // As we provided a Preset ID of 2, the next Preset to use should still be 1
    ASSERT_EQ(server.GetLogic().mCurrentPresetID, 1);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZMoveToPresetCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

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
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Try to move to a preset, this should fail with NOT FOUND
    uint8_t presetIDAsInt = 2;

    moveToPresetCommandData.presetID = presetIDAsInt;
    auto moveResponse = server.GetLogic().HandleMPTZMoveToPreset(commandHandler, kCommandPathMove, moveToPresetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(moveResponse.has_value());
    ASSERT_EQ(moveResponse.value().GetStatusCode().GetStatus(), Status::NotFound);

    // Save the current values of MPTZ as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    savePresetCommandData.name = presetName;
    savePresetCommandData.presetID.Emplace(presetIDAsInt);
    auto saveResponse = server.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPathMPTZSave, savePresetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(saveResponse.has_value());
    EXPECT_TRUE(saveResponse.value().IsSuccess());

    // Save the stored preset settings; we know there is only one entry
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);

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

    auto setResponse = server.GetLogic().HandleMPTZSetPosition(commandHandler, kCommandPathMPTZSet, mptzSetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(setResponse.has_value());
    EXPECT_TRUE(setResponse.value().IsSuccess());

    // Fake the delegate callback
    server.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Verify that the values in the attribute are the values set by the command
    Structs::MPTZStruct::DecodableType mptzPosition;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), testPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), testTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), testZoom);

    // Move to the preset
    moveResponse = server.GetLogic().HandleMPTZMoveToPreset(commandHandler, kCommandPathMove, moveToPresetCommandData).value();

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(moveResponse.has_value());
    EXPECT_TRUE(moveResponse.value().IsSuccess());

    // Ensure we're "moving"
    PhysicalMovementEnum movementState;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kMoving);

    // Fake the callback
    server.GetLogic().OnPhysicalMovementComplete(Status::Success);

    // Ensure we've reverted to idle
    ASSERT_EQ(tester.ReadAttribute(Attributes::MovementState::Id, movementState), CHIP_NO_ERROR);
    ASSERT_EQ(movementState, PhysicalMovementEnum::kIdle);

    // Ensure the values in MPTZPosition are those of the Preset
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPosition::Id, mptzPosition), CHIP_NO_ERROR);
    ASSERT_EQ(mptzPosition.pan.Value(), presetPan);
    ASSERT_EQ(mptzPosition.tilt.Value(), presetTilt);
    ASSERT_EQ(mptzPosition.zoom.Value(), presetZoom);
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteMPTZRemovePresetCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPathRemove{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZRemovePreset::Id };
    Commands::MPTZRemovePreset::DecodableType removePresetCommandData;
    ConcreteCommandPath kCommandPathMPTZSave{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::MPTZSavePreset::Id };
    Commands::MPTZSavePreset::DecodableType savePresetCommandData;

    // Presets attribute should be empty
    Attributes::MPTZPresets::TypeInfo::DecodableType mptzPresets;
    size_t presetsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));

    // Try to remove a preset, this should fail with NOT FOUND
    uint8_t presetIDAsInt = 2;

    removePresetCommandData.presetID = presetIDAsInt;
    auto removeResponse =
        server.GetLogic().HandleMPTZRemovePreset(commandHandler, kCommandPathRemove, removePresetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(removeResponse.has_value());
    ASSERT_EQ(removeResponse.value().GetStatusCode().GetStatus(), Status::NotFound);

    // Save the current values of MPTZ as a preset
    chip::CharSpan presetName("DefaultPreset"_span);
    savePresetCommandData.name = presetName;
    savePresetCommandData.presetID.Emplace(presetIDAsInt);
    auto saveResponse = server.GetLogic().HandleMPTZSavePreset(commandHandler, kCommandPathMPTZSave, savePresetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(saveResponse.has_value());
    EXPECT_TRUE(saveResponse.value().IsSuccess());

    // Verify that there is a single saved preset. The values match those of the current MPTZ Position
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(1));

    // Try to remove again, verify success
    removePresetCommandData.presetID = presetIDAsInt;
    removeResponse = server.GetLogic().HandleMPTZRemovePreset(commandHandler, kCommandPathRemove, removePresetCommandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(removeResponse.has_value());
    EXPECT_TRUE(removeResponse.value().IsSuccess());

    // Verify that there are no saved presets. The values match those of the current MPTZ Position
    ASSERT_EQ(tester.ReadAttribute(Attributes::MPTZPresets::Id, mptzPresets), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED mptzPresets.ComputeSize(&presetsSize);
    ASSERT_EQ(presetsSize, static_cast<size_t>(0));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, ExecuteDPTZSetViewportCommandTest)
{
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::DPTZSetViewport::Id };
    Commands::DPTZSetViewport::DecodableType commandData;

    // DPTZStreams attribute starts empty
    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED dptzStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    // Try to set the viewport when no streams exist, this should fail with NOT FOUND
    uint8_t videoStreamID                           = 1;
    Globals::Structs::ViewportStruct::Type viewPort = { 0, 0, 1920, 1080 };
    commandData.videoStreamID                       = videoStreamID;
    commandData.viewport                            = viewPort;
    auto response = server.GetLogic().HandleDPTZSetViewport(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::NotFound);

    // Allocate a video stream
    server.GetLogic().AddMoveCapableVideoStream(videoStreamID, viewPort);

    // Verify that DPTZStreams is now populated correctly
    ASSERT_EQ(tester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
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

    response = server.GetLogic().HandleDPTZSetViewport(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    EXPECT_TRUE(response.value().IsSuccess());

    // Verify this new viewport is in DPTZStreams
    ASSERT_EQ(tester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);

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
    TestServerClusterContext context;

    // Setup persistent storage
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider provider;
    ASSERT_EQ(provider.Init(&storageDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&provider);

    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<Feature, uint32_t> testFeatures(Feature::kDigitalPTZ, Feature::kMechanicalPan, Feature::kMechanicalTilt,
                                             Feature::kMechanicalZoom, Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(&mockDelegate);

    EXPECT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(server);

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::CameraAvSettingsUserLevelManagement::Id, Commands::DPTZRelativeMove::Id };
    Commands::DPTZRelativeMove::DecodableType commandData;

    // DPTZStreams attribute starts empty
    Attributes::DPTZStreams::TypeInfo::DecodableType dptzStreams;
    size_t streamsSize;
    ASSERT_EQ(tester.ReadAttribute(Attributes::DPTZStreams::Id, dptzStreams), CHIP_NO_ERROR);
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

    auto response = server.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::NotFound);

    // Allocate a video stream
    Globals::Structs::ViewportStruct::Type viewPort = { 0, 0, 1920, 1080 };
    server.GetLogic().AddMoveCapableVideoStream(videoStreamID, viewPort);

    // Try to move the viewport
    response = server.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    EXPECT_TRUE(response.has_value());
    EXPECT_TRUE(response.value().IsSuccess());

    // The mock delegate has no means to verify the proposed change as the values needed are part of CameraAVStreams,
    // as such, the command above should just be successful as long as the delta is in range.

    // Try to move the viewport with an out of range zoomDelta
    commandData.zoomDelta.Emplace(zoomDeltaFail);
    response = server.GetLogic().HandleDPTZRelativeMove(commandHandler, kCommandPath, commandData);
    EXPECT_TRUE(response.has_value());
    ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::ConstraintError);
}

} // namespace
