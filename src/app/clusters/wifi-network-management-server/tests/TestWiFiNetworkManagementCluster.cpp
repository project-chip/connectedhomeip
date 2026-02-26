/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/wifi-network-management-server/WiFiNetworkManagementCluster.h>
#include <pw_unit_test/framework.h>

#include <access/SubjectDescriptor.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WiFiNetworkManagement/Attributes.h>
#include <clusters/WiFiNetworkManagement/Commands.h>
#include <clusters/WiFiNetworkManagement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkManagement;
using namespace chip::app::Clusters::WiFiNetworkManagement::Attributes;
using namespace chip::Testing;

namespace {

struct TestWiFiNetworkManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }

    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestWiFiNetworkManagementCluster() : cluster(kTestEndpointId), tester(cluster) {}

    static constexpr EndpointId kTestEndpointId = 1;

    WiFiNetworkManagementCluster cluster;
    ClusterTester tester;
};

} // namespace

TEST_F(TestWiFiNetworkManagementCluster, AttributeListTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Ssid::kMetadataEntry,
                                            PassphraseSurrogate::kMetadataEntry,
                                        }));
}

TEST_F(TestWiFiNetworkManagementCluster, ReadGlobalAttributesTest)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, WiFiNetworkManagement::kRevision);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features, 0u);
}

TEST_F(TestWiFiNetworkManagementCluster, InitialStateHasNoCredentials)
{
    EXPECT_FALSE(cluster.HasNetworkCredentials());
    EXPECT_TRUE(cluster.Ssid().empty());
    EXPECT_TRUE(cluster.Passphrase().empty());

    DataModel::Nullable<ByteSpan> ssid;
    ASSERT_EQ(tester.ReadAttribute(Ssid::Id, ssid), CHIP_NO_ERROR);
    EXPECT_TRUE(ssid.IsNull());

    DataModel::Nullable<uint64_t> passphraseSurrogate;
    ASSERT_EQ(tester.ReadAttribute(PassphraseSurrogate::Id, passphraseSurrogate), CHIP_NO_ERROR);
    EXPECT_TRUE(passphraseSurrogate.IsNull());
}

TEST_F(TestWiFiNetworkManagementCluster, SetNetworkCredentials)
{
    const uint8_t ssidData[]       = { 'T', 'e', 's', 't', 'S', 'S', 'I', 'D' };
    const uint8_t passphraseData[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3' };
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData)), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasNetworkCredentials());
    EXPECT_TRUE(cluster.Ssid().data_equal(ByteSpan(ssidData)));
    EXPECT_TRUE(cluster.Passphrase().data_equal(ByteSpan(passphraseData)));

    DataModel::Nullable<ByteSpan> ssid;
    ASSERT_EQ(tester.ReadAttribute(Ssid::Id, ssid), CHIP_NO_ERROR);
    ASSERT_FALSE(ssid.IsNull());
    EXPECT_TRUE(ssid.Value().data_equal(ByteSpan(ssidData)));

    DataModel::Nullable<uint64_t> passphraseSurrogate;
    ASSERT_EQ(tester.ReadAttribute(PassphraseSurrogate::Id, passphraseSurrogate), CHIP_NO_ERROR);
    EXPECT_FALSE(passphraseSurrogate.IsNull());

    // Both attributes should have been marked dirty
    EXPECT_TRUE(tester.IsAttributeDirty(Ssid::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(PassphraseSurrogate::Id));

    // Setting the same credentials again should not generate further notifications
    tester.GetDirtyList().clear();
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData)), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.GetDirtyList().empty());
}

TEST_F(TestWiFiNetworkManagementCluster, ClearNetworkCredentials)
{
    const uint8_t ssidData[]       = { 'T', 'e', 's', 't', 'S', 'S', 'I', 'D' };
    const uint8_t passphraseData[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3' };
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData)), CHIP_NO_ERROR);

    // Verify credentials are set
    DataModel::Nullable<ByteSpan> ssid;
    ASSERT_EQ(tester.ReadAttribute(Ssid::Id, ssid), CHIP_NO_ERROR);
    ASSERT_FALSE(ssid.IsNull());

    // Clear credentials
    tester.GetDirtyList().clear();
    ASSERT_EQ(cluster.ClearNetworkCredentials(), CHIP_NO_ERROR);

    EXPECT_FALSE(cluster.HasNetworkCredentials());
    EXPECT_TRUE(cluster.Ssid().empty());
    EXPECT_TRUE(cluster.Passphrase().empty());

    // Verify credentials are cleared
    ASSERT_EQ(tester.ReadAttribute(Ssid::Id, ssid), CHIP_NO_ERROR);
    EXPECT_TRUE(ssid.IsNull());

    DataModel::Nullable<uint64_t> passphraseSurrogate;
    ASSERT_EQ(tester.ReadAttribute(PassphraseSurrogate::Id, passphraseSurrogate), CHIP_NO_ERROR);
    EXPECT_TRUE(passphraseSurrogate.IsNull());

    // Both attributes should have been marked dirty
    EXPECT_TRUE(tester.IsAttributeDirty(Ssid::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(PassphraseSurrogate::Id));

    // Clearing again when already cleared should not generate further notifications
    tester.GetDirtyList().clear();
    ASSERT_EQ(cluster.ClearNetworkCredentials(), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.GetDirtyList().empty());
}

TEST_F(TestWiFiNetworkManagementCluster, SetNetworkCredentialsValidation)
{
    // SSID too short (empty)
    const uint8_t validPassphrase[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd' };
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(), ByteSpan(validPassphrase)), CHIP_ERROR_INVALID_ARGUMENT);

    // SSID too long (> 32 bytes)
    const uint8_t longSsid[33] = {};
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(longSsid), ByteSpan(validPassphrase)), CHIP_ERROR_INVALID_ARGUMENT);

    // Passphrase too short (less than 8 characters)
    const uint8_t ssidData[]      = { 'T', 'e', 's', 't' };
    const uint8_t shortPassword[] = { 's', 'h', 'o', 'r', 't' };
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(shortPassword)), CHIP_ERROR_INVALID_ARGUMENT);

    // Valid credentials should succeed
    const uint8_t validPassword[] = { 'v', 'a', 'l', 'i', 'd', 'p', 'a', 's', 's' };
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(validPassword)), CHIP_NO_ERROR);

    // Valid 64-byte hex PSK
    const uint8_t validHexPsk[64] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
                                      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
                                      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
                                      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(validHexPsk)), CHIP_NO_ERROR);

    // Invalid 64-byte non-hex PSK (contains 'G')
    uint8_t invalidHexPsk[64];
    std::fill(std::begin(invalidHexPsk), std::end(invalidHexPsk), uint8_t('G'));
    EXPECT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(invalidHexPsk)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestWiFiNetworkManagementCluster, PassphraseSurrogateChangesOnPassphraseChange)
{
    const uint8_t ssidData[]        = { 'T', 'e', 's', 't', 'S', 'S', 'I', 'D' };
    const uint8_t passphraseData1[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1' };
    const uint8_t passphraseData2[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '2' };

    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData1)), CHIP_NO_ERROR);

    DataModel::Nullable<uint64_t> surrogate1;
    ASSERT_EQ(tester.ReadAttribute(PassphraseSurrogate::Id, surrogate1), CHIP_NO_ERROR);
    ASSERT_FALSE(surrogate1.IsNull());

    // Change passphrase, surrogate should change
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData2)), CHIP_NO_ERROR);

    DataModel::Nullable<uint64_t> surrogate2;
    ASSERT_EQ(tester.ReadAttribute(PassphraseSurrogate::Id, surrogate2), CHIP_NO_ERROR);
    ASSERT_FALSE(surrogate2.IsNull());

    EXPECT_NE(surrogate1.Value(), surrogate2.Value());
}

TEST_F(TestWiFiNetworkManagementCluster, NetworkPassphraseRequestRequiresCaseSession)
{
    const uint8_t ssidData[]       = { 'T', 'e', 's', 't', 'S', 'S', 'I', 'D' };
    const uint8_t passphraseData[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3' };
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData)), CHIP_NO_ERROR);

    // Test with non-CASE auth mode (e.g., PASE) - should be rejected
    Access::SubjectDescriptor paseDescriptor;
    paseDescriptor.authMode    = Access::AuthMode::kPase;
    paseDescriptor.fabricIndex = 1;
    tester.SetSubjectDescriptor(paseDescriptor);

    Commands::NetworkPassphraseRequest::Type request;
    auto result = tester.Invoke(request);

    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::UnsupportedAccess));
}

TEST_F(TestWiFiNetworkManagementCluster, NetworkPassphraseRequestRequiresCredentials)
{
    // Set up CASE session but don't set credentials
    Access::SubjectDescriptor caseDescriptor;
    caseDescriptor.authMode    = Access::AuthMode::kCase;
    caseDescriptor.fabricIndex = 1;
    tester.SetSubjectDescriptor(caseDescriptor);

    Commands::NetworkPassphraseRequest::Type request;
    auto result = tester.Invoke(request);

    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidInState));
}

TEST_F(TestWiFiNetworkManagementCluster, NetworkPassphraseRequestSuccess)
{
    const uint8_t ssidData[]       = { 'T', 'e', 's', 't', 'S', 'S', 'I', 'D' };
    const uint8_t passphraseData[] = { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3' };
    ASSERT_EQ(cluster.SetNetworkCredentials(ByteSpan(ssidData), ByteSpan(passphraseData)), CHIP_NO_ERROR);

    // Set up CASE session
    Access::SubjectDescriptor caseDescriptor;
    caseDescriptor.authMode    = Access::AuthMode::kCase;
    caseDescriptor.fabricIndex = 1;
    tester.SetSubjectDescriptor(caseDescriptor);

    Commands::NetworkPassphraseRequest::Type request;
    auto result = tester.Invoke(request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.response.value().passphrase.data_equal(ByteSpan(passphraseData)));
}
