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

#pragma once

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include <app/util/af.h>

struct LockUserInfo
{
    char userName[DOOR_LOCK_USER_NAME_BUFFER_SIZE];
    DlCredential credentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
    size_t totalCredentials;
    uint32_t userUniqueId;
    DlUserStatus userStatus;
    DlUserType userType;
    DlCredentialRule credentialRule;
    chip::FabricIndex createdBy;
    chip::FabricIndex lastModifiedBy;
};

static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE = 20;

struct LockCredentialInfo
{
    DlCredentialStatus status;
    DlCredentialType credentialType;
    uint8_t credentialData[DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE];
    size_t credentialDataSize;
};

class LockManager
{
public:
    LockManager() : mLocked(false) {}

    bool Lock(chip::Optional<chip::ByteSpan> pin);
    bool Unlock(chip::Optional<chip::ByteSpan> pin);

    bool GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);
    bool SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                 const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                 DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials);

    bool GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential);

    bool SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                       DlCredentialType credentialType, const chip::ByteSpan & credentialData);

    static LockManager & Instance();

private:
    bool mLocked;

    // TODO: Support multiple endpoints in the app.
    std::array<LockUserInfo, 10> mLockUsers;
    std::array<LockCredentialInfo, 10> mLockCredentials;
    static LockManager instance;
};
