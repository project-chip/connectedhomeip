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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/camera-av-stream-management-server/MigrationCameraAVStreamManagementCluster.h>
#include <app/clusters/camera-av-stream-management-server/MigratorCameraAVStreamManagementStorage.h>
#include <lib/support/CodeUtils.h>

CHIP_ERROR
chip::app::Clusters::CameraAvStreamManagement::MigrationCameraAVStreamManagementCluster::Startup(ServerClusterContext & context)
{
    SafeAttributePersistenceProvider * srcProvider = GetSafeAttributePersistenceProvider();
    if (srcProvider != nullptr)
    {
        LogErrorOnFailure(MigratorCameraAVStreamManagementStorage(mPath.mEndpointId, *srcProvider, context.attributeStorage));
    }
    return CameraAVStreamManagementCluster::Startup(context);
}
