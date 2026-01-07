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
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Attributes.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Commands.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Enums.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

static constexpr chip::EndpointId kTestEndpointId = 1;

// Minimal mock delegate for testing
class MockCameraAvSettingsUserLevelManagementDelegate : public Delegate
{
public:
    void ShutdownApp() {}

    bool CanChangeMPTZ() {return true; }

    void VideoStreamAllocated(uint16_t aStreamID) {}
    void VideoStreamDeallocated(uint16_t aStreamID) {}
    void DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport) {}

    Protocols::InteractionModel::Status
    MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                    CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) {return Status::Success; }

    Protocols::InteractionModel::Status
    MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                     CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) {return Status::Success; }

    Protocols::InteractionModel::Status
    MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                     CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) {return Status::Success; }

    Protocols::InteractionModel::Status MPTZSavePreset(uint8_t aPreset) {return Status::Success; }

    Protocols::InteractionModel::Status MPTZRemovePreset(uint8_t aPreset) {return Status::Success; }

    Protocols::InteractionModel::Status DPTZSetViewport(uint16_t aVideoStreamID,
                                                                Globals::Structs::ViewportStruct::Type aViewport) {return Status::Success; }

    Protocols::InteractionModel::Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                                 Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta,
                                                                 Globals::Structs::ViewportStruct::Type & aViewport) {return Status::Success; }

    virtual CHIP_ERROR PersistentAttributesLoadedCallback() {return CHIP_NO_ERROR; }

    CHIP_ERROR LoadMPTZPresets(std::vector<CameraAvSettingsUserLevelManagement::MPTZPresetHelper> & mptzPresetHelpers) {return CHIP_NO_ERROR; }
    CHIP_ERROR LoadDPTZStreams(std::vector<CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type> & dptzStreams) {return CHIP_NO_ERROR; }
};

struct TestCameraAvSettingsUserLevelManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, TestAttributes)
{
    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<CameraAvSettingsUserLevelManagement::Feature, uint32_t> testFeatures(
        CameraAvSettingsUserLevelManagement::Feature::kDigitalPTZ, CameraAvSettingsUserLevelManagement::Feature::kMechanicalPan,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalTilt,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalZoom,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(mockDelegate);

    ASSERT_TRUE(IsAttributesListEqualTo(server, { 
                                                    CameraAvSettingsUserLevelManagement::Attributes::MPTZPosition::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::MaxPresets::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::MPTZPresets::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::DPTZStreams::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::ZoomMax::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::TiltMin::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::TiltMax::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::PanMin::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::PanMax::kMetadataEntry,
                                                    CameraAvSettingsUserLevelManagement::Attributes::MovementState::kMetadataEntry,
                                                   }));
}

TEST_F(TestCameraAvSettingsUserLevelManagementCluster, TestCommands)
{
    MockCameraAvSettingsUserLevelManagementDelegate mockDelegate;
    BitFlags<CameraAvSettingsUserLevelManagement::Feature, uint32_t> testFeatures(
        CameraAvSettingsUserLevelManagement::Feature::kDigitalPTZ, CameraAvSettingsUserLevelManagement::Feature::kMechanicalPan,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalTilt,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalZoom,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalPresets);

    const uint8_t testMaxPresets = 5;

    CameraAvSettingsUserLevelManagementCluster server(kTestEndpointId, testFeatures, testMaxPresets);
    server.SetDelegate(mockDelegate);

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(server,
                                              {
                                                  CameraAvSettingsUserLevelManagement::Commands::MPTZSetPosition::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::MPTZRelativeMove::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::MPTZMoveToPreset::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::MPTZSavePreset::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::MPTZRemovePreset::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::DPTZSetViewport::kMetadataEntry,
                                                  CameraAvSettingsUserLevelManagement::Commands::DPTZRelativeMove::kMetadataEntry,
                                              }));
}

} // namespace
