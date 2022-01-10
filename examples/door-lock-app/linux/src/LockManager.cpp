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
    mLocked = true;
    return true;
}

bool LockManager::Unlock(chip::Optional<chip::ByteSpan> pin)
{
    mLocked = false;
    return true;
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    userIndex = userIndex - 1;

    if (userIndex > mLockUsers.size())
    {
        return false;
    }

    const auto & userInDb = mLockUsers[userIndex];
    user.userStatus       = userInDb.userStatus;
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName, strlen(userInDb.userName));
    user.credentials    = chip::Span<const DlCredential>(userInDb.credentials, userInDb.totalCredentials);
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    user.createdBy      = userInDb.createdBy;
    user.lastModifiedBy = userInDb.lastModifiedBy;

    return true;
}

bool LockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                          DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    userIndex = userIndex - 1;

    if (userIndex > mLockUsers.size())
    {
        return false;
    }

    auto & userInStorage = mLockUsers[userIndex];

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

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    credentialIndex = credentialIndex - 1;

    if (credentialIndex > mLockCredentials.size())
    {
        return false;
    }

    const auto & credentialInStorage = mLockCredentials[credentialIndex];

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable != credential.status)
    {
        credential.credentialType = credentialInStorage.credentialType;
        credential.credentialData = chip::ByteSpan(credentialInStorage.credentialData, credentialInStorage.credentialDataSize);
    }
    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    credentialIndex = credentialIndex - 1;

    if (credentialIndex > mLockCredentials.size())
    {
        return false;
    }

    auto & credentialInStorage = mLockCredentials[credentialIndex];
    if (credentialData.size() > DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    std::memcpy(credentialInStorage.credentialData, credentialData.data(), credentialData.size());
    credentialInStorage.credentialDataSize = credentialData.size();

    return true;
}
