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

#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <app/clusters/camera-av-stream-management-server/MigratorCameraAVStreamManagementStorage.h>
#include <app/persistence/AttributePersistenceMigration.h>
#include <clusters/CameraAvStreamManagement/Ids.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;

CHIP_ERROR CameraAvStreamManagement::MigratorCameraAVStreamManagementStorage(EndpointId endpointId,
                                                                             SafeAttributePersistenceProvider & safeProvider,
                                                                             AttributePersistenceProvider & dstProvider)
{
    static constexpr AttrMigrationData kAttributesToMigrate[] = {
        { Attributes::HDRModeEnabled::Id, sizeof(bool), true },
        { Attributes::SoftRecordingPrivacyModeEnabled::Id, sizeof(bool), true },
        { Attributes::SoftLivestreamPrivacyModeEnabled::Id, sizeof(bool), true },
        { Attributes::NightVision::Id, sizeof(uint8_t), true },
        { Attributes::NightVisionIllum::Id, sizeof(uint8_t), true },
        { Attributes::SpeakerMuted::Id, sizeof(bool), true },
        { Attributes::SpeakerVolumeLevel::Id, sizeof(uint8_t), true },
        { Attributes::MicrophoneMuted::Id, sizeof(bool), true },
        { Attributes::MicrophoneVolumeLevel::Id, sizeof(uint8_t), true },
        { Attributes::MicrophoneAGCEnabled::Id, sizeof(bool), true },
        { Attributes::ImageRotation::Id, sizeof(uint16_t), true },
        { Attributes::ImageFlipHorizontal::Id, sizeof(bool), true },
        { Attributes::ImageFlipVertical::Id, sizeof(bool), true },
        { Attributes::LocalVideoRecordingEnabled::Id, sizeof(bool), true },
        { Attributes::LocalSnapshotRecordingEnabled::Id, sizeof(bool), true },
        { Attributes::StatusLightEnabled::Id, sizeof(bool), true },
        { Attributes::StatusLightBrightness::Id, sizeof(uint8_t), true },
        { Attributes::Viewport::Id, kViewportStructMaxSerializedSize, false },
        { Attributes::StreamUsagePriorities::Id, kStreamUsagePrioritiesTlvSize, false },
    };

    static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(kAttributesToMigrate);
    static_assert(kBufferSize > 0, "All migration attributes have zero valueSize");

    uint8_t attributeBuffer[kBufferSize];
    MutableByteSpan buffer(attributeBuffer);

    return MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider,
                                                         ConcreteClusterPath(endpointId, CameraAvStreamManagement::Id),
                                                         Span<const AttrMigrationData>(kAttributesToMigrate), buffer);
}
