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

#include <lib/core/PeerId.h>
#include <lib/dnssd/Discovery_ImplPlatform.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/fake/DnssdImpl.h>

#include <nlunit-test.h>

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
OperationalAdvertisingParameters operationalParams2 =
    OperationalAdvertisingParameters()
        .SetPeerId(kPeerId2)
        .SetMac(ByteSpan(kMac))
        .SetPort(CHIP_PORT)
        .EnableIpV4(true)
        .SetLocalMRPConfig(Optional<ReliableMessageProtocolConfig>::Value(32_ms32, 30_ms32, 10_ms16)) // SII and SAI to match below
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
        .SetVendorId(Optional<uint16_t>(555))
        .SetDeviceType(Optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledBasic)
        .SetDeviceName(Optional<const char *>("testy-test"))
        .SetPairingHint(Optional<uint16_t>(3))
        .SetPairingInstruction(Optional<const char *>("Pair me"))
        .SetProductId(Optional<uint16_t>(897))
        .SetRotatingDeviceId(Optional<const char *>("id_that_spins"))
        .SetICDModeToAdvertise(ICDModeAdvertise::kSIT)
        // 3600005 is over the max, so this should be adjusted by the platform
        .SetLocalMRPConfig(Optional<ReliableMessageProtocolConfig>::Value(3600000_ms32, 3600005_ms32, 65535_ms16));

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
        .SetVendorId(chip::Optional<uint16_t>(555))
        .SetDeviceType(chip::Optional<uint32_t>(70000))
        .SetCommissioningMode(CommissioningMode::kEnabledEnhanced)
        .SetDeviceName(chip::Optional<const char *>("testy-test"))
        .SetPairingHint(chip::Optional<uint16_t>(3))
        .SetPairingInstruction(chip::Optional<const char *>("Pair me"))
        .SetProductId(chip::Optional<uint16_t>(897))
        .SetRotatingDeviceId(chip::Optional<const char *>("id_that_spins"));

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
void TestStub(nlTestSuite * inSuite, void * inContext)
{
    // This is a test of the fake platform impl. We want
    // We want the platform to return unexpected event if it gets a start
    // without an expected event.
    ChipLogError(Discovery, "Test platform returns error correctly");
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Init(DeviceLayer::UDPEndPointManager()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.RemoveServices() == CHIP_NO_ERROR);
    OperationalAdvertisingParameters params;
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(params) == CHIP_ERROR_UNEXPECTED_EVENT);
}

void TestOperational(nlTestSuite * inSuite, void * inContext)
{
    ChipLogError(Discovery, "Test operational");
    test::Reset();
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Init(DeviceLayer::UDPEndPointManager()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.RemoveServices() == CHIP_NO_ERROR);

    operationalCall1.callType = test::CallType::kStart;
    NL_TEST_ASSERT(inSuite, test::AddExpectedCall(operationalCall1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(operationalParams1) == CHIP_NO_ERROR);

    // Next call to advertise should call start again with just the new data.
    test::Reset();
    operationalCall2.callType = test::CallType::kStart;
    NL_TEST_ASSERT(inSuite, test::AddExpectedCall(operationalCall2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(operationalParams2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, mdnsPlatform.FinalizeServiceUpdate() == CHIP_NO_ERROR);
}

void TestCommissionableNode(nlTestSuite * inSuite, void * inContext)
{
    ChipLogError(Discovery, "Test commissionable");
    test::Reset();
    DiscoveryImplPlatform & mdnsPlatform = DiscoveryImplPlatform::GetInstance();
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Init(DeviceLayer::UDPEndPointManager()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.RemoveServices() == CHIP_NO_ERROR);

    commissionableSmall.callType = test::CallType::kStart;
    NL_TEST_ASSERT(inSuite,
                   mdnsPlatform.GetCommissionableInstanceName(commissionableSmall.instanceName,
                                                              sizeof(commissionableSmall.instanceName)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test::AddExpectedCall(commissionableSmall) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(commissionableNodeParamsSmall) == CHIP_NO_ERROR);

    // TODO: Right now, platform impl doesn't stop commissionable node before starting a new one. Add stop call here once that is
    // fixed.
    test::Reset();
    commissionableLargeBasic.callType = test::CallType::kStart;
    NL_TEST_ASSERT(inSuite,
                   mdnsPlatform.GetCommissionableInstanceName(commissionableLargeBasic.instanceName,
                                                              sizeof(commissionableLargeBasic.instanceName)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test::AddExpectedCall(commissionableLargeBasic) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(commissionableNodeParamsLargeBasic) == CHIP_NO_ERROR);

    test::Reset();
    commissionableLargeEnhanced.callType = test::CallType::kStart;
    NL_TEST_ASSERT(inSuite,
                   mdnsPlatform.GetCommissionableInstanceName(commissionableLargeEnhanced.instanceName,
                                                              sizeof(commissionableLargeEnhanced.instanceName)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, test::AddExpectedCall(commissionableLargeEnhanced) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mdnsPlatform.Advertise(commissionableNodeParamsLargeEnhanced) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, mdnsPlatform.FinalizeServiceUpdate() == CHIP_NO_ERROR);
}

int TestSetup(void * inContext)
{
    return chip::Platform::MemoryInit() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int TestTeardown(void * inContext)
{
    DiscoveryImplPlatform::GetInstance().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestStub", TestStub),                             //
    NL_TEST_DEF("TestOperational", TestOperational),               //
    NL_TEST_DEF("TestCommissionableNode", TestCommissionableNode), //
    NL_TEST_SENTINEL()                                             //
};

} // namespace

int TestDnssdPlatform()
{
    nlTestSuite theSuite = { "DnssdPlatform", &sTests[0], &TestSetup, &TestTeardown };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDnssdPlatform)
