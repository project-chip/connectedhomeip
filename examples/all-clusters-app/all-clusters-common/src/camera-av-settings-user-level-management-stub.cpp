/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <camera-av-settings-user-level-management-instance.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using chip::Protocols::InteractionModel::Status;

static AVSettingsUserLevelManagementDelegate * gDelegate = nullptr;
static CameraAvSettingsUserLevelMgmtServer * gAVSettingsUserLevelManagementCluster = nullptr;
static constexpr EndpointId kEndpointId = 1;

CameraAvSettingsUserLevelMgmtServer * GetInstance()
{
    return gAVSettingsUserLevelManagementCluster;
}


void Shutdown()
{
    if (gAVSettingsUserLevelManagementCluster != nullptr)
    {
        delete gAVSettingsUserLevelManagementCluster;
        gAVSettingsUserLevelManagementCluster = nullptr;
    }
}


//AVSettingsUserLevelManagementDelegate::AVSettingsUserLevelManagementDelegate() {}

Status AVSettingsUserLevelManagementDelegate::PersistentAttributesLoadedCallback()
{
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZSetPosition() 
{
   return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRelativeMove()
{
  return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZMoveToPreset() 
{
  return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZSavePreset()
{
  return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRemovePreset()
{
  return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZSetViewport()
{
  return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZRelativeMove()
{
  return Status::Success;
}

void emberAfCameraAvSettingsUserLevelManagementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gDelegate == nullptr && gAVSettingsUserLevelManagementCluster == nullptr);

    gDelegate = new AVSettingsUserLevelManagementDelegate;
    BitMask<CameraAvSettingsUserLevelManagement::Feature, uint32_t> avsumFeatures(CameraAvSettingsUserLevelManagement::Feature::kDigitalPTZ, 
       CameraAvSettingsUserLevelManagement::Feature::kMechanicalPan, CameraAvSettingsUserLevelManagement::Feature::kMechanicalTilt,
       CameraAvSettingsUserLevelManagement::Feature::kMechanicalZoom, CameraAvSettingsUserLevelManagement::Feature::kMechanicalPresets);
    gAVSettingsUserLevelManagementCluster = new CameraAvSettingsUserLevelMgmtServer(kEndpointId, gDelegate, avsumFeatures);
    gAVSettingsUserLevelManagementCluster->Init();
}