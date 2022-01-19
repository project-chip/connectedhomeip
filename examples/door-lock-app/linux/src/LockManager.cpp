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

#include "LockManager.h"

#include <cstring>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

LockManager LockManager::instance;

LockManager & LockManager::Instance()
{
    return instance;
}

bool LockManager::Lock(chip::Optional<chip::ByteSpan> pin)
{
    return setLockState(DlLockState::kLocked, pin);
}

bool LockManager::Unlock(chip::Optional<chip::ByteSpan> pin)
{
    return setLockState(DlLockState::kUnlocked, pin);
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    uint16_t adjustedUserIndex = userIndex - 1;
    if (adjustedUserIndex > mLockUsers.size())
    {
        ChipLogError(Zcl, "Cannot get user - index out of range [endpoint=%d,index=%hu,adjustedIndex=%d]", endpointId, userIndex,
                     adjustedUserIndex);
        return false;
    }

    const auto & userInDb = mLockUsers[adjustedUserIndex];
    user.userStatus       = userInDb.userStatus;
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d,adjustedIndex=%hu]", endpointId, adjustedUserIndex);
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName, strlen(userInDb.userName));
    user.credentials    = chip::Span<const DlCredential>(userInDb.credentials, userInDb.totalCredentials);
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    user.createdBy      = userInDb.createdBy;
    user.lastModifiedBy = userInDb.lastModifiedBy;

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,adjustedIndex=%hu,name=\"%*.s\",credentialsCount=%zu,uniqueId=%x,type=%hhu,credentialRule=%hhu,"
                  "createdBy=%d,lastModifiedBy=%d]",
                  endpointId, adjustedUserIndex, static_cast<int>(user.userName.size()), user.userName.data(),
                  user.credentials.size(), user.userUniqueId, user.userType, user.credentialRule, user.createdBy,
                  user.lastModifiedBy);

    return true;
}

bool LockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                          DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetUser "
                    "[endpoint=%d,userIndex=%hu,creator=%d,modifier=%d,userName=\"%*.s\",uniqueId=%x,userStatus=%hhu,userType=%hhu,"
                    "credentialRule=%hhu,credentials=%p,totalCredentials=%zu]",
                    endpointId, userIndex, creator, modifier, static_cast<int>(userName.size()), userName.data(), uniqueId,
                    userStatus, usertype, credentialRule, credentials, totalCredentials);

    uint16_t adjustedUserIndex = userIndex - 1;
    if (adjustedUserIndex > mLockUsers.size())
    {
        ChipLogError(Zcl, "Cannot set user - index out of range [endpoint=%d,index=%d,adjustedUserIndex=%hu]", endpointId,
                     userIndex, adjustedUserIndex);
        return false;
    }

    auto & userInStorage = mLockUsers[adjustedUserIndex];

    strncpy(userInStorage.userName, userName.data(), userName.size());
    userInStorage.userUniqueId   = uniqueId;
    userInStorage.userStatus     = userStatus;
    userInStorage.userType       = usertype;
    userInStorage.credentialRule = credentialRule;
    userInStorage.lastModifiedBy = modifier;
    userInStorage.createdBy      = creator;

    userInStorage.totalCredentials = totalCredentials;
    for (size_t i = 0; i < totalCredentials; ++i)
    {
        userInStorage.credentials[i] = credentials[i];
    }

    ChipLogProgress(Zcl, "Successfully set the user [endpointId=%d,index=%d,adjustedIndex=%d]", endpointId, userIndex,
                    adjustedUserIndex);

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetCredential [endpoint=%d,credentialIndex=%hu,credentialType=%hhu]",
                    endpointId, credentialIndex, credentialType);

    if (credentialIndex >= mLockCredentials.size() || (0 == credentialIndex && DlCredentialType::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [endpoint=%d,index=%d]", endpointId, credentialIndex);
        return false;
    }

    const auto & credentialInStorage = mLockCredentials[credentialIndex];

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential [endpoint=%d,index=%hu]", endpointId, credentialIndex);
        return true;
    }
    credential.credentialType = credentialInStorage.credentialType;
    credential.credentialData = chip::ByteSpan(credentialInStorage.credentialData, credentialInStorage.credentialDataSize);

    ChipLogDetail(Zcl, "Found occupied credential [endpoint=%d,index=%hu,type=%hhu,dataSize=%zu]", endpointId, credentialIndex,
                  credential.credentialType, credential.credentialData.size());

    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetCredential "
                    "[endpoint=%d,credentialIndex=%hu,credentialStatus=%hhu,credentialType=%hhu,credentialDataSize=%zu]",
                    endpointId, credentialIndex, credentialStatus, credentialType, credentialData.size());

    if (credentialIndex >= mLockCredentials.size() || (0 == credentialIndex && DlCredentialType::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot set the credential - index out of range [endpoint=%d,index=%d]", endpointId, credentialIndex);
        return false;
    }

    auto & credentialInStorage = mLockCredentials[credentialIndex];
    if (credentialData.size() > DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        ChipLogError(Zcl,
                     "Cannot get the credential - data size exceeds limit "
                     "[endpoint=%d,index=%d,dataSize=%zu,maxDataSize=%zu]",
                     endpointId, credentialIndex, credentialData.size(), DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE);
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    std::memcpy(credentialInStorage.credentialData, credentialData.data(), credentialData.size());
    credentialInStorage.credentialDataSize = credentialData.size();

    ChipLogProgress(Zcl, "Successfully set the credential [endpointId=%d,index=%d,credentialType=%hhu]",
                    endpointId, credentialIndex, credentialType);

    return true;
}

bool LockManager::setLockState(DlLockState lockState, chip::Optional<chip::ByteSpan> & pin)
{
    if (mLocked == lockState)
    {
        ChipLogDetail(Zcl, "Door Lock App: door is already locked, ignoring command to set lock state to \"%s\"",
                      lockStateToString(lockState));
        return true;
    }

    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified, setting door lock state to \"%s\"",
                      lockStateToString(lockState));
        mLocked = lockState;
        return true;
    }

    // Check the PIN code
    for (const auto & pinCredential : mLockCredentials)
    {
        if (pinCredential.credentialType != DlCredentialType::kPin || pinCredential.status == DlCredentialStatus::kAvailable)
        {
            continue;
        }

        chip::ByteSpan credentialData(pinCredential.credentialData, pinCredential.credentialDataSize);
        if (credentialData.data_equal(pin.Value()))
        {
            ChipLogDetail(Zcl, "Door Lock App: specified PIN code was found in the database, setting door lock state to \"%s\"",
                          lockStateToString(lockState));

            mLocked = lockState;
            return true;
        }
    }

    ChipLogDetail(Zcl,
                  "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\"",
                  lockStateToString(lockState));

    return false;
}

const char * LockManager::lockStateToString(DlLockState lockState)
{
    switch (lockState)
    {
    case DlLockState::kNotFullyLocked:
        return "Not Fully Locked";
    case DlLockState::kLocked:
        return "Locked";
    case DlLockState::kUnlocked:
        return "Unlocked";
    }

    return "Unknown";
}
