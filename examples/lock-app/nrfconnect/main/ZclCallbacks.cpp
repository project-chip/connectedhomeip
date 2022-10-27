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
#include <lib/support/CodeUtils.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    VerifyOrReturn(attributePath.mClusterId == DoorLock::Id && attributePath.mAttributeId == DoorLock::Attributes::LockState::Id);

    switch (*value)
    {
    case to_underlying(DlLockState::kLocked):
        BoltLockMgr().Lock(BoltLockManager::OperationSource::kRemote);
        break;
    case to_underlying(DlLockState::kUnlocked):
        BoltLockMgr().Unlock(BoltLockManager::OperationSource::kRemote);
        break;
    default:
        break;
    }
}

bool emberAfPluginDoorLockGetUser(EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return BoltLockMgr().GetUser(userIndex, user);
}

bool emberAfPluginDoorLockSetUser(EndpointId endpointId, uint16_t userIndex, FabricIndex creator, FabricIndex modifier,
                                  const CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType userType,
                                  DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    return BoltLockMgr().SetUser(userIndex, creator, modifier, userName, uniqueId, userStatus, userType, credentialRule,
                                 credentials, totalCredentials);
}

bool emberAfPluginDoorLockGetCredential(EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return BoltLockMgr().GetCredential(credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(EndpointId endpointId, uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                        DlCredentialStatus credentialStatus, DlCredentialType credentialType,
                                        const ByteSpan & secret)
{
    return BoltLockMgr().SetCredential(credentialIndex, creator, modifier, credentialStatus, credentialType, secret);
}

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    bool result = BoltLockMgr().ValidatePIN(pinCode, err);

    /* Handle changing attribute state on command reception */
    if (result)
    {
        BoltLockMgr().Lock(BoltLockManager::OperationSource::kRemote);
    }

    return result;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
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
    DoorLockServer::Instance().InitServer(endpoint);

    const auto logOnFailure = [](EmberAfStatus status, const char * attributeName) {
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(Zcl, "Failed to set DoorLock %s: %x", attributeName, status);
        }
    };

    logOnFailure(DoorLock::Attributes::LockType::Set(endpoint, DlLockType::kDeadBolt), "type");
    logOnFailure(DoorLock::Attributes::NumberOfTotalUsersSupported::Set(endpoint, CONFIG_LOCK_NUM_USERS), "number of users");
    logOnFailure(DoorLock::Attributes::NumberOfPINUsersSupported::Set(endpoint, CONFIG_LOCK_NUM_USERS), "number of PIN users");
    logOnFailure(DoorLock::Attributes::NumberOfRFIDUsersSupported::Set(endpoint, 0), "number of RFID users");
    logOnFailure(DoorLock::Attributes::NumberOfCredentialsSupportedPerUser::Set(endpoint, CONFIG_LOCK_NUM_CREDENTIALS_PER_USER),
                 "number of credentials per user");

    // Set FeatureMap to (kUsersManagement|kPINCredentials), default is:
    // (kUsersManagement|kAccessSchedules|kRFIDCredentials|kPINCredentials) 0x113
    logOnFailure(DoorLock::Attributes::FeatureMap::Set(endpoint, 0x101), "feature map");

    GetAppTask().UpdateClusterState(BoltLockMgr().GetState(), BoltLockManager::OperationSource::kUnspecified);
}
