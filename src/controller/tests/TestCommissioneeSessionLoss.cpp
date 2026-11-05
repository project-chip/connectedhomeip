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

#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/DefaultSessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SessionHolder.h>

#include <memory>

using namespace chip;
using namespace chip::Controller;

namespace {

constexpr NodeId kCommissioneeNodeId = 0x1234;

class NoNOCCredentials : public OperationalCredentialsDelegate
{
    CHIP_ERROR GenerateNOCChain(const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &,
                                const ByteSpan &, Callback::Callback<OnNOCChainGeneration> *) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

class RecordingPairingDelegate : public DevicePairingDelegate
{
public:
    void OnPASESessionLost(NodeId deviceId) override
    {
        lostCount++;
        lostDeviceId = deviceId;
    }
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override { commissioningCompleteCount++; }

    int lostCount                  = 0;
    NodeId lostDeviceId            = kUndefinedNodeId;
    int commissioningCompleteCount = 0;
};

class TestCommissioneeSessionLoss : public ::testing::Test
{
public:
    struct State
    {
        TestPersistentStorageDelegate storage;
        Credentials::PersistentStorageOpCertStore certificates;
        Crypto::DefaultSessionKeystore keys;
        Credentials::GroupDataProviderImpl groups;
        NoNOCCredentials credentials;
        RecordingPairingDelegate pairingDelegate;
        DeviceCommissioner commissioner;
    };

    void SetUp() override
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        state = std::make_unique<State>();
        ASSERT_EQ(state->certificates.Init(&state->storage), CHIP_NO_ERROR);
        state->groups.SetStorageDelegate(&state->storage);
        state->groups.SetSessionKeystore(&state->keys);
        ASSERT_EQ(state->groups.Init(), CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&state->groups);

        FactoryInitParams factory;
        factory.fabricIndependentStorage = &state->storage;
        factory.opCertStore              = &state->certificates;
        factory.sessionKeystore          = &state->keys;
        factory.groupDataProvider        = &state->groups;
        factory.dataModelProvider        = app::CodegenDataModelProviderInstance(&state->storage);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().Init(factory), CHIP_NO_ERROR);
        DeviceControllerFactory::GetInstance().RetainSystemState();

        SetupParams setup;
        setup.controllerVendorId             = VendorId::TestVendor1;
        setup.operationalCredentialsDelegate = &state->credentials;
        setup.deviceAttestationVerifier      = Credentials::GetDefaultDACVerifier(nullptr);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(setup, state->commissioner), CHIP_NO_ERROR);
        state->commissioner.RegisterPairingDelegate(&state->pairingDelegate);
    }

    void TearDown() override
    {
        state->commissioner.Shutdown();
        DeviceControllerFactory::GetInstance().ReleaseSystemState();
        DeviceControllerFactory::GetInstance().Shutdown();
        Credentials::SetGroupDataProvider(nullptr);
        state->groups.Finish();
        state.reset();
        Platform::MemoryShutdown();
    }

    // Leaves a commissionee paired over PASE with no commissioning started: the state after OnPairingComplete when the
    // client has not yet called Commission().
    CommissioneeDeviceProxy * PairWithoutCommissioning(SessionHolder & sessionHolder)
    {
        Testing::DeviceCommissionerTestAccess access(&state->commissioner);
        auto peer     = Transport::PeerAddress::UDP(Inet::IPAddress::Loopback(Inet::IPAddressType::kIPv6), 5540);
        auto * device = access.StartPASEEstablishment(kCommissioneeNodeId, peer);
        if (device == nullptr)
        {
            return nullptr;
        }
        CHIP_ERROR err = access.GetSessionManager()->InjectPaseSessionWithTestKey(
            sessionHolder, /* localSessionId */ 1, kCommissioneeNodeId, /* peerSessionId */ 2, kUndefinedFabricIndex, peer,
            CryptoContext::SessionRole::kInitiator);
        if (err != CHIP_NO_ERROR)
        {
            return nullptr;
        }
        access.CompletePASE(sessionHolder.Get().Value());
        return device;
    }

    // Runs the work already queued (including any deferred release), then returns.
    void DrainWork()
    {
        ASSERT_EQ(DeviceLayer::PlatformMgr().ScheduleWork(
                      [](intptr_t) { EXPECT_EQ(DeviceLayer::PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

    std::unique_ptr<State> state;
};

TEST_F(TestCommissioneeSessionLoss, ReleasesIdleCommissioneeAndNotifiesWhenPASESessionIsReleased)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    SessionHolder sessionHolder;
    ASSERT_NE(PairWithoutCommissioning(sessionHolder), nullptr);
    sessionHolder.Release();
    ASSERT_NE(access.FindCommissioneeDevice(kCommissioneeNodeId), nullptr);

    access.GetSessionManager()->ExpireAllPASESessions();
    DrainWork();

    EXPECT_EQ(access.FindCommissioneeDevice(kCommissioneeNodeId), nullptr);
    EXPECT_EQ(state->pairingDelegate.lostCount, 1);
    EXPECT_EQ(state->pairingDelegate.lostDeviceId, kCommissioneeNodeId);
    EXPECT_EQ(state->pairingDelegate.commissioningCompleteCount, 0);
}

TEST_F(TestCommissioneeSessionLoss, StopPairingAfterPASESessionLossFindsNothingToStop)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    SessionHolder sessionHolder;
    ASSERT_NE(PairWithoutCommissioning(sessionHolder), nullptr);
    sessionHolder.Release();

    access.GetSessionManager()->ExpireAllPASESessions();
    DrainWork();

    EXPECT_EQ(state->commissioner.StopPairing(kCommissioneeNodeId), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    EXPECT_EQ(state->pairingDelegate.lostCount, 1);
}

TEST_F(TestCommissioneeSessionLoss, CommissioneeWithLivePASESessionIsKept)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    SessionHolder sessionHolder;
    ASSERT_NE(PairWithoutCommissioning(sessionHolder), nullptr);
    sessionHolder.Release();

    DrainWork();

    EXPECT_NE(access.FindCommissioneeDevice(kCommissioneeNodeId), nullptr);
    EXPECT_EQ(state->pairingDelegate.lostCount, 0);
}

} // namespace
