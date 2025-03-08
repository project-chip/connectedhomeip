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

#include <app-common/zap-generated/ids/Attributes.h>
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

constexpr ClusterId kNetworkCommissioningCluster = app::Clusters::NetworkCommissioning::Id;
constexpr ClusterId kDescriptorCluster           = app::Clusters::Descriptor::Id;
constexpr ClusterId kOnOffCluster                = app::Clusters::OnOff::Id;

// Clusters allowed to have restrictions
constexpr ClusterId kWiFiNetworkManagementCluster  = app::Clusters::WiFiNetworkManagement::Id;
constexpr ClusterId kThreadBorderRouterMgmtCluster = app::Clusters::ThreadBorderRouterManagement::Id;
constexpr ClusterId kThreadNetworkDirectoryCluster = app::Clusters::ThreadNetworkDirectory::Id;

constexpr NodeId kOperationalNodeId1 = 0x1111111111111111;
constexpr NodeId kOperationalNodeId2 = 0x2222222222222222;
constexpr NodeId kOperationalNodeId3 = 0x3333333333333333;

bool operator==(const AccessRestrictionProvider::Restriction & lhs, const AccessRestrictionProvider::Restriction & rhs)
{
    return lhs.restrictionType == rhs.restrictionType && lhs.id == rhs.id;
}

bool operator==(const AccessRestrictionProvider::Entry & lhs, const AccessRestrictionProvider::Entry & rhs)
{
    return lhs.fabricIndex == rhs.fabricIndex && lhs.endpointNumber == rhs.endpointNumber && lhs.clusterId == rhs.clusterId &&
        lhs.restrictions == rhs.restrictions;
}

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
constexpr size_t aclEntryDataCount = MATTER_ARRAY_SIZE(aclEntryData);

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
        CHIP_ERROR expectedResult = checkData[i].allow ? CHIP_NO_ERROR : CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL;
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

// ensure failure when adding restrictons on endpoint 0 (any cluster, including those allowed on other endpoints)
TEST_F(TestAccessRestriction, InvalidRestrictionsOnEndpointZeroTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.endpointNumber = 0;
    entry.fabricIndex    = 1;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });

    entry.clusterId = kDescriptorCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    entries.clear();
    entry.clusterId = kNetworkCommissioningCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    entries.clear();
    entry.clusterId = kWiFiNetworkManagementCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    entries.clear();
    entry.clusterId = kThreadBorderRouterMgmtCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    entries.clear();
    entry.clusterId = kThreadNetworkDirectoryCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);

    // also test a cluster on endpoint 0 that isnt in the special allowed list
    entries.clear();
    entry.clusterId = kOnOffCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);
}

// ensure no failure adding restrictions on endpoint 1 for allowed clusters only:
// wifi network management, thread border router, thread network directory
TEST_F(TestAccessRestriction, ValidRestrictionsOnEndpointOneTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.endpointNumber = 1;
    entry.fabricIndex    = 1;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });

    entry.clusterId = kWiFiNetworkManagementCluster;
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);

    entries.clear();
    entry.clusterId = kThreadBorderRouterMgmtCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);

    entries.clear();
    entry.clusterId = kThreadNetworkDirectoryCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);

    // also test a cluster on endpoint 1 that isnt in the special allowed list
    entries.clear();
    entry.clusterId = kOnOffCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestAccessRestriction, InvalidRestrictionsOnEndpointOneTest)
{
    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry;
    entry.endpointNumber = 1;
    entry.fabricIndex    = 1;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });
    entry.clusterId = kOnOffCluster;
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_ERROR_INVALID_ARGUMENT);
}

constexpr CheckData accessAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
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
    entry.clusterId      = kWiFiNetworkManagementCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(accessAttributeRestrictionTestData, MATTER_ARRAY_SIZE(accessAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(accessAttributeRestrictionTestData, MATTER_ARRAY_SIZE(accessAttributeRestrictionTestData));
}

constexpr CheckData writeAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
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
    entry.clusterId      = kWiFiNetworkManagementCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeWriteForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(writeAttributeRestrictionTestData, MATTER_ARRAY_SIZE(writeAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(writeAttributeRestrictionTestData, MATTER_ARRAY_SIZE(writeAttributeRestrictionTestData));
}

constexpr CheckData commandAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
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
    entry.clusterId      = kWiFiNetworkManagementCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(commandAttributeRestrictionTestData, MATTER_ARRAY_SIZE(commandAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(commandAttributeRestrictionTestData, MATTER_ARRAY_SIZE(commandAttributeRestrictionTestData));
}

constexpr CheckData eventAttributeRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
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
    entry.clusterId      = kWiFiNetworkManagementCluster;
    entry.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kEventForbidden });

    // test wildcarded entity id
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(eventAttributeRestrictionTestData, MATTER_ARRAY_SIZE(eventAttributeRestrictionTestData));

    // test specific entity id
    entries.clear();
    entry.restrictions[0].id.SetValue(1);
    entries.push_back(entry);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);
    RunChecks(eventAttributeRestrictionTestData, MATTER_ARRAY_SIZE(eventAttributeRestrictionTestData));
}

constexpr CheckData combinedRestrictionTestData[] = {
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 3 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 4 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 3 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 4 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kEventReadRequest,
                             .entityId    = 5 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeWriteRequest,
                             .entityId    = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

TEST_F(TestAccessRestriction, CombinedRestrictionTest)
{
    // a restriction for all access to attribute 1 and 2, attributes 3 and 4 are allowed
    std::vector<AccessRestrictionProvider::Entry> entries1;
    AccessRestrictionProvider::Entry entry1;
    entry1.fabricIndex    = 1;
    entry1.endpointNumber = 1;
    entry1.clusterId      = kWiFiNetworkManagementCluster;
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
    entry2.clusterId      = kWiFiNetworkManagementCluster;
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });
    entry2.restrictions[0].id.SetValue(1);
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });
    entry2.restrictions[1].id.SetValue(2);
    entries2.push_back(entry2);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(2, entries2), CHIP_NO_ERROR);

    RunChecks(combinedRestrictionTestData, MATTER_ARRAY_SIZE(combinedRestrictionTestData));
}

TEST_F(TestAccessRestriction, AttributeStorageSeperationTest)
{
    std::vector<AccessRestrictionProvider::Entry> commissioningEntries;
    AccessRestrictionProvider::Entry entry1;
    entry1.fabricIndex    = 1;
    entry1.endpointNumber = 1;
    entry1.clusterId      = kWiFiNetworkManagementCluster;
    entry1.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeWriteForbidden });
    entry1.restrictions[0].id.SetValue(1);
    commissioningEntries.push_back(entry1);
    EXPECT_EQ(accessRestrictionProvider.SetCommissioningEntries(commissioningEntries), CHIP_NO_ERROR);

    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry2;
    entry2.fabricIndex    = 2;
    entry2.endpointNumber = 2;
    entry2.clusterId      = kThreadBorderRouterMgmtCluster;
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kCommandForbidden });
    entry2.restrictions[0].id.SetValue(2);
    entries.push_back(entry2);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(2, entries), CHIP_NO_ERROR);

    auto commissioningEntriesFetched = accessRestrictionProvider.GetCommissioningEntries();
    std::vector<AccessRestrictionProvider::Entry> arlEntriesFetched;
    EXPECT_EQ(accessRestrictionProvider.GetEntries(2, arlEntriesFetched), CHIP_NO_ERROR);
    EXPECT_EQ(commissioningEntriesFetched[0], entry1);
    EXPECT_EQ(commissioningEntriesFetched.size(), static_cast<size_t>(1));
    EXPECT_EQ(arlEntriesFetched[0], entry2);
    EXPECT_EQ(arlEntriesFetched.size(), static_cast<size_t>(1));
    EXPECT_FALSE(commissioningEntriesFetched[0] == arlEntriesFetched[0]);
}

constexpr CheckData listSelectionDuringCommissioningData[] = {
    { .subjectDescriptor = { .fabricIndex     = 1,
                             .authMode        = AuthMode::kCase,
                             .subject         = kOperationalNodeId1,
                             .isCommissioning = true },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex     = 1,
                             .authMode        = AuthMode::kCase,
                             .subject         = kOperationalNodeId1,
                             .isCommissioning = true },
      .requestPath       = { .cluster     = kThreadBorderRouterMgmtCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex     = 1,
                             .authMode        = AuthMode::kCase,
                             .subject         = kOperationalNodeId1,
                             .isCommissioning = false },
      .requestPath       = { .cluster     = kWiFiNetworkManagementCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex     = 1,
                             .authMode        = AuthMode::kCase,
                             .subject         = kOperationalNodeId1,
                             .isCommissioning = false },
      .requestPath       = { .cluster     = kThreadBorderRouterMgmtCluster,
                             .endpoint    = 1,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = 1 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
};

TEST_F(TestAccessRestriction, ListSelectiondDuringCommissioningTest)
{
    // during commissioning, read is allowed on WifiNetworkManagement and disallowed on ThreadBorderRouterMgmt
    // after commissioning, read is disallowed on WifiNetworkManagement and allowed on ThreadBorderRouterMgmt

    std::vector<AccessRestrictionProvider::Entry> entries;
    AccessRestrictionProvider::Entry entry1;
    entry1.fabricIndex    = 1;
    entry1.endpointNumber = 1;
    entry1.clusterId      = kThreadBorderRouterMgmtCluster;
    entry1.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });
    entry1.restrictions[0].id.SetValue(1);
    entries.push_back(entry1);
    EXPECT_EQ(accessRestrictionProvider.SetCommissioningEntries(entries), CHIP_NO_ERROR);

    entries.clear();
    AccessRestrictionProvider::Entry entry2;
    entry2.fabricIndex    = 1;
    entry2.endpointNumber = 1;
    entry2.clusterId      = kWiFiNetworkManagementCluster;
    entry2.restrictions.push_back({ .restrictionType = AccessRestrictionProvider::Type::kAttributeAccessForbidden });
    entry2.restrictions[0].id.SetValue(1);
    entries.push_back(entry2);
    EXPECT_EQ(accessRestrictionProvider.SetEntries(1, entries), CHIP_NO_ERROR);

    RunChecks(listSelectionDuringCommissioningData, MATTER_ARRAY_SIZE(listSelectionDuringCommissioningData));
}

} // namespace Access
} // namespace chip
