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

#include <pw_unit_test/framework.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <lib/support/Span.h>

#include <cstring>

// This suite is built with DOOR_LOCK_USE_LOCAL_BUFFER=1 (see BUILD.gn), which is never
// enabled in the default configuration: it exercises the copy semantics of the
// local-buffer variants of EmberAfPluginDoorLockUserInfo/EmberAfPluginDoorLockCredentialInfo,
// whose spans must be re-bound to the destination's own buffers on copy.
static_assert(DOOR_LOCK_USE_LOCAL_BUFFER == 1, "This test exercises the local-buffer variant of the structs");

namespace {

using chip::MutableByteSpan;
using chip::MutableCharSpan;

constexpr char kUserName[]          = "Alice";
constexpr size_t kUserNameLength    = sizeof(kUserName) - 1;
constexpr uint8_t kCredentialData[] = { 0x01, 0x02, 0x03, 0x04 };

void FillUser(EmberAfPluginDoorLockUserInfo & user)
{
    memcpy(user.nameBuffer, kUserName, kUserNameLength);
    user.userName             = MutableCharSpan(user.nameBuffer, kUserNameLength);
    user.credentialsBuffer[0] = { CredentialTypeEnum::kPin, 3 };
    user.credentialsBuffer[1] = { CredentialTypeEnum::kRfid, 7 };
    user.credentials          = chip::Span<CredentialStruct>(user.credentialsBuffer, 2);
    user.userUniqueId         = 0xdeadbeef;
    user.userStatus           = UserStatusEnum::kOccupiedEnabled;
    user.userType             = UserTypeEnum::kUnrestrictedUser;
    user.credentialRule       = CredentialRuleEnum::kSingle;
    user.creationSource       = DlAssetSource::kMatterIM;
    user.createdBy            = 1;
    user.modificationSource   = DlAssetSource::kMatterIM;
    user.lastModifiedBy       = 2;
}

void FillCredential(EmberAfPluginDoorLockCredentialInfo & credential)
{
    memcpy(credential.credentialDataBuffer, kCredentialData, sizeof(kCredentialData));
    credential.credentialData     = MutableByteSpan(credential.credentialDataBuffer, sizeof(kCredentialData));
    credential.status             = DlCredentialStatus::kOccupied;
    credential.credentialType     = CredentialTypeEnum::kPin;
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.createdBy          = 1;
    credential.modificationSource = DlAssetSource::kMatterIM;
    credential.lastModifiedBy     = 2;
}

void CheckUserCopy(const EmberAfPluginDoorLockUserInfo & copy)
{
    // The spans must be re-bound to the copy's own buffers, not the source's.
    EXPECT_EQ(copy.userName.data(), copy.nameBuffer);
    EXPECT_EQ(copy.credentials.data(), copy.credentialsBuffer);

    ASSERT_EQ(copy.userName.size(), kUserNameLength);
    EXPECT_EQ(memcmp(copy.userName.data(), kUserName, kUserNameLength), 0);
    ASSERT_EQ(copy.credentials.size(), 2u);
    EXPECT_EQ(copy.credentials[0].credentialType, CredentialTypeEnum::kPin);
    EXPECT_EQ(copy.credentials[0].credentialIndex, 3);
    EXPECT_EQ(copy.credentials[1].credentialType, CredentialTypeEnum::kRfid);
    EXPECT_EQ(copy.credentials[1].credentialIndex, 7);

    EXPECT_EQ(copy.userUniqueId, 0xdeadbeef);
    EXPECT_EQ(copy.userStatus, UserStatusEnum::kOccupiedEnabled);
    EXPECT_EQ(copy.userType, UserTypeEnum::kUnrestrictedUser);
    EXPECT_EQ(copy.credentialRule, CredentialRuleEnum::kSingle);
    EXPECT_EQ(copy.creationSource, DlAssetSource::kMatterIM);
    EXPECT_EQ(copy.createdBy, 1);
    EXPECT_EQ(copy.modificationSource, DlAssetSource::kMatterIM);
    EXPECT_EQ(copy.lastModifiedBy, 2);
}

TEST(TestDoorLockLocalBufferCopy, UserCopyAssignmentRebindsSpans)
{
    EmberAfPluginDoorLockUserInfo user;
    FillUser(user);

    EmberAfPluginDoorLockUserInfo copy;
    copy = user;
    CheckUserCopy(copy);

    // Wiping the source must not affect the copy: with the implicitly-defined
    // copy the spans would still alias the source's buffers.
    memset(user.nameBuffer, 0, sizeof(user.nameBuffer));
    user.credentialsBuffer[0] = { CredentialTypeEnum::kProgrammingPIN, 0 };
    CheckUserCopy(copy);
}

TEST(TestDoorLockLocalBufferCopy, UserCopyConstructorRebindsSpans)
{
    EmberAfPluginDoorLockUserInfo user;
    FillUser(user);

    EmberAfPluginDoorLockUserInfo copy(user);
    CheckUserCopy(copy);

    memset(user.nameBuffer, 0, sizeof(user.nameBuffer));
    CheckUserCopy(copy);
}

TEST(TestDoorLockLocalBufferCopy, UserSelfAssignmentIsSafe)
{
    EmberAfPluginDoorLockUserInfo user;
    FillUser(user);

    EmberAfPluginDoorLockUserInfo & alias = user;
    user                                  = alias;
    CheckUserCopy(user);
}

TEST(TestDoorLockLocalBufferCopy, UserCopyHandlesEmptyAndNullSpans)
{
    EmberAfPluginDoorLockUserInfo user;
    // A default-constructed span has a null data pointer; the copy must not
    // pass it to memcpy and must still re-bind to the destination's buffers.
    user.userName    = MutableCharSpan();
    user.credentials = chip::Span<CredentialStruct>();

    EmberAfPluginDoorLockUserInfo copy;
    copy = user;
    EXPECT_EQ(copy.userName.size(), 0u);
    EXPECT_EQ(copy.userName.data(), copy.nameBuffer);
    EXPECT_EQ(copy.credentials.size(), 0u);
    EXPECT_EQ(copy.credentials.data(), copy.credentialsBuffer);
}

TEST(TestDoorLockLocalBufferCopy, CredentialCopyAssignmentRebindsSpan)
{
    EmberAfPluginDoorLockCredentialInfo credential;
    FillCredential(credential);

    EmberAfPluginDoorLockCredentialInfo copy;
    copy = credential;

    EXPECT_EQ(copy.credentialData.data(), copy.credentialDataBuffer);
    ASSERT_EQ(copy.credentialData.size(), sizeof(kCredentialData));
    EXPECT_EQ(memcmp(copy.credentialData.data(), kCredentialData, sizeof(kCredentialData)), 0);
    EXPECT_EQ(copy.status, DlCredentialStatus::kOccupied);
    EXPECT_EQ(copy.credentialType, CredentialTypeEnum::kPin);
    EXPECT_EQ(copy.creationSource, DlAssetSource::kMatterIM);
    EXPECT_EQ(copy.createdBy, 1);
    EXPECT_EQ(copy.modificationSource, DlAssetSource::kMatterIM);
    EXPECT_EQ(copy.lastModifiedBy, 2);

    memset(credential.credentialDataBuffer, 0xff, sizeof(credential.credentialDataBuffer));
    EXPECT_EQ(memcmp(copy.credentialData.data(), kCredentialData, sizeof(kCredentialData)), 0);
}

TEST(TestDoorLockLocalBufferCopy, CredentialCopyConstructorRebindsSpan)
{
    EmberAfPluginDoorLockCredentialInfo credential;
    FillCredential(credential);

    EmberAfPluginDoorLockCredentialInfo copy(credential);
    EXPECT_EQ(copy.credentialData.data(), copy.credentialDataBuffer);
    ASSERT_EQ(copy.credentialData.size(), sizeof(kCredentialData));
    EXPECT_EQ(memcmp(copy.credentialData.data(), kCredentialData, sizeof(kCredentialData)), 0);
}

TEST(TestDoorLockLocalBufferCopy, CredentialSelfAssignmentAndEmptySpanAreSafe)
{
    EmberAfPluginDoorLockCredentialInfo credential;
    FillCredential(credential);

    EmberAfPluginDoorLockCredentialInfo & alias = credential;
    credential                                  = alias;
    EXPECT_EQ(credential.credentialData.data(), credential.credentialDataBuffer);
    ASSERT_EQ(credential.credentialData.size(), sizeof(kCredentialData));
    EXPECT_EQ(memcmp(credential.credentialData.data(), kCredentialData, sizeof(kCredentialData)), 0);

    EmberAfPluginDoorLockCredentialInfo empty;
    empty.credentialData = MutableByteSpan();

    EmberAfPluginDoorLockCredentialInfo copy;
    copy = empty;
    EXPECT_EQ(copy.credentialData.size(), 0u);
    EXPECT_EQ(copy.credentialData.data(), copy.credentialDataBuffer);
}

} // namespace
