/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include "BoltLockManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    LOG_INF("Locking");
    BoltLockMgr().InitiateAction(0, BoltLockManager::LOCK_ACTION);
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    LOG_INF("Unlocking");
    BoltLockMgr().InitiateAction(0, BoltLockManager::UNLOCK_ACTION);
    return true;
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    DoorLockServer::Instance().InitServer(endpoint);
    EmberAfStatus status = DoorLock::Attributes::LockType::Set(endpoint, DlLockType::kDeadBolt);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating type %x", status);
    }
    // Set FeatureMap to 0, defaults is
    //(kUsersManagement|kAccessSchedules|kRFIDCredentials|kPINCredentials) 0x113
    status = DoorLock::Attributes::FeatureMap::Set(endpoint, 0);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating type %x", status);
    }
    GetAppTask().UpdateClusterState();
}
