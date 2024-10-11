/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/PeerId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Discovery_ImplPlatform.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/fake/DnssdImpl.h>

#if CHIP_DEVICE_LAYER_TARGET_FAKE != 1
#error "This test is designed for use only with the fake platform"
#endif

namespace {
using namespace chip;
using namespace chip::Dnssd;

const uint8_t kMac[kMaxMacSize] = { 1, 2, 3, 4, 5, 6, 7, 8 };
const char host[]               = "0102030405060708";

const PeerId kPeerId1 = PeerId().SetCompressedFabricId(0xBEEFBEEFF00DF00D).SetNodeId(0x1111222233334444);
const PeerId kPeerId2 = PeerId().SetCompressedFabricId(0x5555666677778888).SetNodeId(0x1212343456567878);
OperationalAdvertisingParameters operationalParams1 =
    OperationalAdvertisingParameters().SetPeerId(kPeerId1).SetMac(ByteSpan(kMac)).SetPort(CHIP_PORT).EnableIpV4(true);
test::ExpectedCall operationalCall1 = test::ExpectedCall()
                                          .SetProtocol(DnssdServiceProtocol::kDnssdProtocolTcp)
                                          .SetServiceName("_matter")
                                          .SetInstanceName("BEEFBEEFF00DF00D-1111222233334444")
                                          .SetHostName(host)
                                          .AddSubtype("_IBEEFBEEFF00DF00D");
OperationalAdvertisingParameters operationalParams2 = OperationalAdvertisingParameters()
                                                          .SetPeerId(kPeerId2)
                                                          .SetMac(ByteSpan(kMac))
                                                          .SetPort(CHIP_PORT)
                                                          .EnableIpV4(true)
                                                          .SetLocalMRPConfig(std::make_optional<ReliableMessageProtocolConfig>(
                                                              32_ms32, 30_ms32, 10_ms16)) // SII and SAI to match below
                                                          .SetICDModeToAdvertise(ICDModeAdvertise::kSIT);
test::ExpectedCall operationalCall2 = test::ExpectedCall()
                                          .SetProtocol(DnssdServiceProtocol::kDnssdProtocolTcp)
                                          .SetServiceName("_matter")
                                          .SetInstanceName("5555666677778888-1212343456567878")
                                          .SetHostName(host)
                                          .AddSubtype("_I5555666677778888")
                                          .AddTxt("SII", "32")
                                          .AddTxt("SAI", "30")
                                          .AddTxt("SAT", "10")
                                          .AddTxt("ICD", "0");

CommissionAdvertisingParameters commissionableNodeParamsSmall =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac))
        .SetLongDiscriminator(0xFFE)
        .SetShortDiscriminator(0xF)
        .SetCommissioningMode(CommissioningMode::kDisabled);
// Instance names need to be obtained from the advertiser, so they are not set here.
test::ExpectedCall commissionableSmall = test::ExpectedCall()
                                             .SetProtocol(DnssdServiceProtocol::kDnssdProtocolUdp)
                                             .SetServiceName("_matterc")
                                             .SetHostName(host)
                                             .AddTxt("CM", "0")
                                             .AddTxt("D", "4094")
                                             .AddSubtype("_S15")
                                             .AddSubtype("_L4094");
CommissionAdvertisingParameters commissionableNodeParamsLargeBasic =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac, sizeof(kMac)))
        .SetLongDiscriminator(22)
        .SetShortDiscriminator(2)
        .SetVendorId(std::make_optional<uint16_t>(555))
        .SetDeviceType(std::make_optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledBasic)
        .SetDeviceName(std::make_optional<const char *>("testy-test"))
        .SetPairingHint(std::make_optional<uint16_t>(3))
        .SetPairingInstruction(std::make_optional<const char *>("Pair me"))
        .SetProductId(std::make_optional<uint16_t>(897))
        .SetRotatingDeviceId(std::make_optional<const char *>("id_that_spins"))
        .SetICDModeToAdvertise(ICDModeAdvertise::kSIT)
        // 3600005 is over the max, so this should be adjusted by the platform
        .SetLocalMRPConfig(std::make_optional<ReliableMessageProtocolConfig>(3600000_ms32, 3600005_ms32, 65535_ms16));

test::ExpectedCall commissionableLargeBasic = test::ExpectedCall()
                                                  .SetProtocol(DnssdServiceProtocol::kDnssdProtocolUdp)
                                                  .SetServiceName("_matterc")
                                                  .SetHostName(host)
                                                  .AddTxt("D", "22")
                                                  .AddTxt("VP", "555+897")
                                                  .AddTxt("CM", "1")
                                                  .AddTxt("DT", "70000")
                                                  .AddTxt("DN", "testy-test")
                                                  .AddTxt("RI", "id_that_spins")
                                                  .AddTxt("PI", "Pair me")
                                                  .AddTxt("PH", "3")
                                                  .AddTxt("ICD", "0")
                                                  .AddTxt("SII", "3600000")
                                                  .AddTxt("SAI", "3600000")
                                                  .AddTxt("SAT", "65535")
                                                  .AddSubtype("_S2")
                                                  .AddSubtype("_L22")
                                                  .AddSubtype("_V555")
                                                  .AddSubtype("_T70000")
                                                  .AddSubtype("_CM");
CommissionAdvertisingParameters commissionableNodeParamsLargeEnhanced =
    CommissionAdvertisingParameters()
        .SetCommissionAdvertiseMode(CommssionAdvertiseMode::kCommissionableNode)
        .SetMac(ByteSpan(kMac, sizeof(kMac)))
        .SetLongDiscriminator(22)
        .SetShortDiscriminator(2)
        .SetVendorId(std::make_optional<uint16_t>(555))
        .SetDeviceType(std::make_optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledEnhanced)
        .SetDeviceName(std::make_optional<const char *>("testy-test"))
        .SetPairingHint(std::make_optional<uint16_t>(3))
        .SetPairingInstruction(std::make_optional<const char *>("Pair me"))
        .SetProductId(std::make_optional<uint16_t>(897))
        .SetRotatingDeviceId(std::make_optional<const char *>("id_that_spins"));

test::ExpectedCall commissionableLargeEnhanced = test::ExpectedCall()
                                                     .SetProtocol(DnssdServiceProtocol::kDnssdProtocolUdp)
                                                     .SetServiceName("_matterc")
                                                     .SetHostName(host)
                                                     .AddTxt("D", "22")
                                                     .AddTxt("VP", "555+897")
                                                     .AddTxt("CM", "2")
                                                     .AddTxt("DT", "70000")
                                                     .AddTxt("DN", "testy-test")
                                                     .AddTxt("RI", "id_that_spins")
                                                     .AddTxt("PI", "Pair me")
                                                     .AddTxt("PH", "3")
                                                     .AddSubtype("_S2")
                                                     .AddSubtype("_L22")
                                                     .AddSubtype("_V555")
                                                     .AddSubtype("_T70000")
                                                     .AddSubtype("_CM");

class TestDnssdPlatform : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
        EXPECT_EQ(mdnsPlatform.Init(DeviceLayer::UDPEndPointManager()), CHIP_NO_ERROR);
        EXPECT_EQ(mdnsPlatform.RemoveServices(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DiscoveryImplPlatform::GetInstance().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void TearDown() override { test::Reset(); }
};

TEST_F(TestDnssdPlatform, TestStub)
{
    // This is a test of the fake platform impl. We want
    // We want the platform to return unexpected event if it gets a start
    // without an expected event.
    ChipLogError(Discovery, "Test platform returns error correctly");
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
    OperationalAdvertisingParameters params;
    EXPECT_EQ(mdnsPlatform.Advertise(params), CHIP_ERROR_UNEXPECTED_EVENT);
}

TEST_F(TestDnssdPlatform, TestOperational)
{
    ChipLogError(Discovery, "Test operational");
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();

    operationalCall1.callType = test::CallType::kStart;
    EXPECT_EQ(test::AddExpectedCall(operationalCall1), CHIP_NO_ERROR);
    EXPECT_EQ(mdnsPlatform.Advertise(operationalParams1), CHIP_NO_ERROR);

    // Next call to advertise should call start again with just the new data.
    test::Reset();
    operationalCall2.callType = test::CallType::kStart;
    EXPECT_EQ(test::AddExpectedCall(operationalCall2), CHIP_NO_ERROR);
    EXPECT_EQ(mdnsPlatform.Advertise(operationalParams2), CHIP_NO_ERROR);

    EXPECT_EQ(mdnsPlatform.FinalizeServiceUpdate(), CHIP_NO_ERROR);
}

TEST_F(TestDnssdPlatform, TestCommissionableNode)
{
    ChipLogError(Discovery, "Test commissionable");
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();

    commissionableSmall.callType = test::CallType::kStart;
    EXPECT_EQ(
        mdnsPlatform.GetCommissionableInstanceName(commissionableSmall.instanceName, sizeof(commissionableSmall.instanceName)),
        CHIP_NO_ERROR);
    EXPECT_EQ(test::AddExpectedCall(commissionableSmall), CHIP_NO_ERROR);
    EXPECT_EQ(mdnsPlatform.Advertise(commissionableNodeParamsSmall), CHIP_NO_ERROR);

    // TODO: Right now, platform impl doesn't stop commissionable node before starting a new one. Add stop call here once that is
    // fixed.
    test::Reset();
    commissionableLargeBasic.callType = test::CallType::kStart;
    EXPECT_EQ(mdnsPlatform.GetCommissionableInstanceName(commissionableLargeBasic.instanceName,
                                                         sizeof(commissionableLargeBasic.instanceName)),
              CHIP_NO_ERROR);
    EXPECT_EQ(test::AddExpectedCall(commissionableLargeBasic), CHIP_NO_ERROR);
    EXPECT_EQ(mdnsPlatform.Advertise(commissionableNodeParamsLargeBasic), CHIP_NO_ERROR);

    test::Reset();
    commissionableLargeEnhanced.callType = test::CallType::kStart;
    EXPECT_EQ(mdnsPlatform.GetCommissionableInstanceName(commissionableLargeEnhanced.instanceName,
                                                         sizeof(commissionableLargeEnhanced.instanceName)),
              CHIP_NO_ERROR);
    EXPECT_EQ(test::AddExpectedCall(commissionableLargeEnhanced), CHIP_NO_ERROR);
    EXPECT_EQ(mdnsPlatform.Advertise(commissionableNodeParamsLargeEnhanced), CHIP_NO_ERROR);

    EXPECT_EQ(mdnsPlatform.FinalizeServiceUpdate(), CHIP_NO_ERROR);
}

} // namespace
