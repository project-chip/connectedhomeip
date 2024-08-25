/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "access/AccessControl.h"
#include "access/AccessRestrictionProvider.h"
#include "access/examples/ExampleAccessControlDelegate.h"

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>

namespace chip {
namespace Access {

class TestAccessRestrictionProvider : public AccessRestrictionProvider
{
    CHIP_ERROR DoRequestFabricRestrictionReview(const FabricIndex fabricIndex, uint64_t token, const std::vector<Entry> & arl)
    {
        return CHIP_NO_ERROR;
    }
};

AccessControl accessControl;
TestAccessRestrictionProvider accessRestrictionProvider;

constexpr ClusterId kNetworkCommissioningCluster = 0x0000'0031; // must not be blocked by access restrictions on any endpoint
constexpr ClusterId kDescriptorCluster           = 0x0000'001d; // must not be blocked by access restrictions on any endpoint
constexpr ClusterId kOnOffCluster                = 0x0000'0006;

constexpr NodeId kOperationalNodeId1 = 0x1111111111111111;
constexpr NodeId kOperationalNodeId2 = 0x2222222222222222;
constexpr NodeId kOperationalNodeId3 = 0x3333333333333333;

struct AclEntryData
{
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    Privilege privilege     = Privilege::kView;
    AuthMode authMode       = AuthMode::kNone;
    NodeId subject;
};

constexpr AclEntryData aclEntryData[] = {
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subject     = kOperationalNodeId1,
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subject     = kOperationalNodeId2,
    },
};
constexpr size_t aclEntryDataCount = ArraySize(aclEntryData);

struct CheckData
{
    SubjectDescriptor subjectDescriptor;
    RequestPath requestPath;
    Privilege privilege;
    bool allow;
};

constexpr CheckData checkDataNoRestrictions[] = {
    // Checks for implicit PASE
    { .subjectDescriptor = { .fabricIndex = 0, .authMode = AuthMode::kPase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 0, .authMode = AuthMode::kPase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kEventReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    // Checks for entry 0
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kEventReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    // Checks for entry 1
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = 1, .endpoint = 1, .requestType = RequestType::kEventReadRequest, .entityId = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

CHIP_ERROR LoadEntry(AccessControl::Entry & entry, const AclEntryData & entryData)
{
    ReturnErrorOnFailure(entry.SetAuthMode(entryData.authMode));
    ReturnErrorOnFailure(entry.SetFabricIndex(entryData.fabricIndex));
    ReturnErrorOnFailure(entry.SetPrivilege(entryData.privilege));
    ReturnErrorOnFailure(entry.AddSubject(nullptr, entryData.subject));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoadAccessControl(AccessControl & ac, const AclEntryData * entryData, size_t count)
{
    AccessControl::Entry entry;
    for (size_t i = 0; i < count; ++i, ++entryData)
    {
        ReturnErrorOnFailure(ac.PrepareEntry(entry));
        ReturnErrorOnFailure(LoadEntry(entry, *entryData));
        ReturnErrorOnFailure(ac.CreateEntry(nullptr, entry));
    }
    return CHIP_NO_ERROR;
}

void RunChecks(const CheckData * checkData, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        CHIP_ERROR expectedResult = checkData[i].allow ? CHIP_NO_ERROR : CHIP_ERROR_ACCESS_DENIED;
        EXPECT_EQ(accessControl.Check(checkData[i].subjectDescriptor, checkData[i].requestPath, checkData[i].privilege),
                  expectedResult);
    }
}

class DeviceTypeResolver : public AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return false; }
} testDeviceTypeResolver;

class TestAccessRestriction : public ::testing::Test
{
public: // protected
    void SetUp() override
    {
        accessRestrictionProvider.SetCommissioningEntries(std::vector<AccessRestrictionProvider::Entry>());
        accessRestrictionProvider.SetEntries(0, std::vector<AccessRestrictionProvider::Entry>());
        accessRestrictionProvider.SetEntries(1, std::vector<AccessRestrictionProvider::Entry>());
        accessRestrictionProvider.SetEntries(2, std::vector<AccessRestrictionProvider::Entry>());
    }

    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        AccessControl::Delegate * delegate = Examples::GetAccessControlDelegate();
        SetAccessControl(accessControl);
        GetAccessControl().SetAccessRestrictionProvider(&accessRestrictionProvider);
        VerifyOrDie(GetAccessControl().Init(delegate, testDeviceTypeResolver) == CHIP_NO_ERROR);
        EXPECT_EQ(LoadAccessControl(accessControl, aclEntryData, aclEntryDataCount), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        GetAccessControl().Finish();
        ResetAccessControlToDefault();
    }
};

// basic data check without restrictions
TEST_F(TestAccessRestriction, MetaTest)
{
    for (const auto & checkData : checkDataNoRestrictions)
    {
        CHIP_ERROR expectedResult = checkData.allow ? CHIP_NO_ERROR : CHIP_ERROR_ACCESS_DENIED;
        EXPECT_EQ(accessControl.Check(checkData.subjectDescriptor, checkData.requestPath, checkData.privilege), expectedResult);
    }
}

// ensure adding restrictons on endpoint 0 (any cluster) or for network commissioning and descriptor clusters fail
TEST_F(TestAccessRestriction, InvalidRestrictionsTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.fabricIndex = 1;
    entry.clusterId   = kOnOffCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });

    // must not restrict endpoint 0
    entry.endpointNumber = 0;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    // must not restrict network commissioning cluster
    entries.clear();
    entry.endpointNumber = 1;
    entry.clusterId      = kNetworkCommissioningCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    // must not restrict descriptor cluster
    entries.clear();
    entry.clusterId = kDescriptorCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);
}

constexpr CheckData accessAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kOnOffCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

TEST_F(TestAccessRestriction, AccessAttributeRestrictionTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.fabricIndex    = 1;
    entry.endpointNumber = 1;
    entry.clusterId      = kOnOffCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(accessAttributeRestrictionTestData, ArraySize(accessAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(accessAttributeRestrictionTestData, ArraySize(accessAttributeRestrictionTestData));
}

constexpr CheckData writeAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kOnOffCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

TEST_F(TestAccessRestriction, WriteAttributeRestrictionTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.fabricIndex    = 1;
    entry.endpointNumber = 1;
    entry.clusterId      = kOnOffCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeWriteForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(writeAttributeRestrictionTestData, ArraySize(writeAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(writeAttributeRestrictionTestData, ArraySize(writeAttributeRestrictionTestData));
}

constexpr CheckData commandAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kOnOffCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

TEST_F(TestAccessRestriction, CommandRestrictionTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.fabricIndex    = 1;
    entry.endpointNumber = 1;
    entry.clusterId      = kOnOffCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(commandAttributeRestrictionTestData, ArraySize(commandAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(commandAttributeRestrictionTestData, ArraySize(commandAttributeRestrictionTestData));
}

constexpr CheckData eventAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kOnOffCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
};

TEST_F(TestAccessRestriction, EventRestrictionTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.fabricIndex    = 1;
    entry.endpointNumber = 1;
    entry.clusterId      = kOnOffCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kEventForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(eventAttributeRestrictionTestData, ArraySize(eventAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(eventAttributeRestrictionTestData, ArraySize(eventAttributeRestrictionTestData));
}

constexpr CheckData combinedRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 2 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 3 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 4 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeReadRequest, .entityId = 3 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 4 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kOnOffCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 5 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kCommandInvokeRequest, .entityId = 1 },
      .privilege   = Privilege::kAdminister,
      .allow       = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath = { .cluster = kOnOffCluster, .endpoint = 1, .requestType = RequestType::kAttributeWriteRequest, .entityId = 2 },
      .privilege   = Privilege::kAdminister,
      .allow       = true },
};

TEST_F(TestAccessRestriction, CombinedRestrictionTest)
{
    // a restriction for all access to attribute 1 and 2, attributes 3 and 4 are allowed
    std::vector<AccessRestrictionProvider::Entry> entries1;
    AccessRestrictionProvider::Entry entry1;
    entry1.fabricIndex    = 1;
    entry1.endpointNumber = 1;
    entry1.clusterId      = kOnOffCluster;
    entry1.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeWriteForbidden });
    entry1.restrictions[0].id.SetValue(1);
    entry1.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });
    entry1.restrictions[1].id.SetValue(2);
    entries1.push_back(entry1);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries1), CHIP_NO_ERROR);

    // a restriction for fabric 2 that forbids command 1 and 2.  Check that command 1 is blocked on invoke, but attribute 2 write is
    // allowed
    std::vector<AccessRestrictionProvider::Entry> entries2;
    AccessRestrictionProvider::Entry entry2;
    entry2.fabricIndex    = 2;
    entry2.endpointNumber = 1;
    entry2.clusterId      = kOnOffCluster;
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });
    entry2.restrictions[0].id.SetValue(1);
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });
    entry2.restrictions[1].id.SetValue(2);
    entries2.push_back(entry2);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(2, entries2), CHIP_NO_ERROR);

    RunChecks(combinedRestrictionTestData, ArraySize(combinedRestrictionTestData));
}

} // namespace Access
} // namespace chip
