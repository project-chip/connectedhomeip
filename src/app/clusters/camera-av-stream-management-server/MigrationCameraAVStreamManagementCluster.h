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

#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

/**
 * Migration integration subclass for CameraAVStreamManagementCluster.
 *
 * This class extends the core cluster with one-time migration logic that runs
 * during Startup(): it attempts to migrate any attribute data previously stored
 * via SafeAttributePersistenceProvider into the standard AttributePersistenceProvider,
 * then delegates to the base Startup(). Persistent attributes are loaded at the end of the base Startup().
 */
class MigrationCameraAVStreamManagementCluster : public CameraAVStreamManagementCluster
{
public:
    using CameraAVStreamManagementCluster::CameraAVStreamManagementCluster;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
