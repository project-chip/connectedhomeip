/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"
#include "LockManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>


using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                       uint16_t size, uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == DoorLock::Id && attributeId == DoorLock::Attributes::LockState::Id)
    {
        LockMgr().InitiateAction(AppEvent::kEventType_Lock, ((*value)-1) ? LockManager::LOCK_ACTION : LockManager::UNLOCK_ACTION);
    }
    
}

/** @brief DoorLock Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions

}

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Unlock Command endpoint=%d", endpointId);
    return true;
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    ChipLogProgress(Zcl, "Door Lock App: Get Credential [endpoint=%d, credentialIndex=%d]", endpointId, credentialIndex);
    return true;
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                        DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    ChipLogProgress(Zcl, "Door Lock App: Set Credential [endpoint=%d, credentialIndex=%d]", endpointId, credentialIndex);
    return true;
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    ChipLogProgress(Zcl, "Door Lock App: Get User [endpoint=%d, userIndex=%d]", endpointId, userIndex);
    return true;
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule,
                                  const DlCredential * credentials, size_t totalCredentials)
{
    ChipLogProgress(Zcl, "Door Lock App: Set User [endpoint=%d, userIndex=%d]", endpointId, userIndex);
    return true;
}