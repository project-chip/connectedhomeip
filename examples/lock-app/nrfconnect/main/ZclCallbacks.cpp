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
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

#include <zephyr/logging/log.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;
using ::chip::app::DataModel::Nullable;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // Door lock state changes are actuated by the lock operation delegate
    // hooks now; attribute writes no longer drive the hardware.
    (void) attributePath;
    (void) type;
    (void) size;
    (void) value;
}

bool emberAfPluginDoorLockGetUser(EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return BoltLockMgr().GetUser(userIndex, user);
}

bool emberAfPluginDoorLockSetUser(EndpointId endpointId, uint16_t userIndex, FabricIndex creator, FabricIndex modifier,
                                  const CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType,
                                  CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    return BoltLockMgr().SetUser(userIndex, creator, modifier, userName, uniqueId, userStatus, userType, credentialRule,
                                 credentials, totalCredentials);
}

bool emberAfPluginDoorLockGetCredential(EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return BoltLockMgr().GetCredential(credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(EndpointId endpointId, uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                        DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                        const ByteSpan & secret)
{
    return BoltLockMgr().SetCredential(credentialIndex, creator, modifier, credentialStatus, credentialType, secret);
}

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    bool result = BoltLockMgr().ValidatePIN(pinCode, err);

    /* Handle changing attribute state on command reception */
    if (result)
    {
        BoltLockMgr().Lock(BoltLockManager::OperationSource::kRemote);
    }

    return result;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    bool result = BoltLockMgr().ValidatePIN(pinCode, err);

    /* Handle changing attribute state on command reception */
    if (result)
    {
        BoltLockMgr().Unlock(BoltLockManager::OperationSource::kRemote);
    }

    return result;
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    // Cluster registration is handled by MatterDoorLockClusterInitCallback;
    // feature/capacity overrides are installed from AppTask::Init.
    DoorLockServer::Instance().InitServer(endpoint);

    // Only the initial lock state is synced from the (already running) hardware.
    AppTask::Instance().UpdateClusterState(BoltLockMgr().GetState(), BoltLockManager::OperationSource::kUnspecified);
}
