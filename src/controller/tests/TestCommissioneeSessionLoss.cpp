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
#include <controller/tests/SetUpCodePairerTestAccess.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/DefaultSessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/IPAddress.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SecureSession.h>
#include <transport/SessionHolder.h>
#include <transport/SessionManager.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

using namespace chip;
using namespace chip::Controller;

namespace {

constexpr NodeId kNodeA = 0x1234;
constexpr NodeId kNodeB = 0x5678;
constexpr NodeId kNodeC = 0x9abc;

constexpr uint16_t kPortA = 5541;
constexpr uint16_t kPortB = 5542;

// The reason reported when an idle commissionee's PASE session is released.
const CHIP_ERROR kSessionReleasedReason = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;

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
    using DevicePairingDelegate::OnPairingComplete;

    void OnPairingComplete(CHIP_ERROR error) override { pairingCompletes.push_back(error); }
    void OnCommissioneeReleased(NodeId deviceId, CHIP_ERROR reason) override
    {
        released.push_back({ deviceId, reason });
        if (onReleased)
        {
            onReleased(deviceId, reason);
        }
    }
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override
    {
        commissioningCompletes.push_back({ deviceId, error });
    }

    struct Release
    {
        NodeId deviceId;
        CHIP_ERROR reason;
    };
    struct Completion
    {
        NodeId deviceId;
        CHIP_ERROR error;
    };

    size_t ReleasedCount(NodeId deviceId) const
    {
        return static_cast<size_t>(std::count_if(released.begin(), released.end(),
                                                 [deviceId](const Release & entry) { return entry.deviceId == deviceId; }));
    }

    std::vector<CHIP_ERROR> pairingCompletes;
    std::vector<Release> released;
    std::vector<Completion> commissioningCompletes;
    // Runs inside OnCommissioneeReleased, for checks that must hold at the time of the call.
    std::function<void(NodeId, CHIP_ERROR)> onReleased;
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
        // Run anything still queued while the commissioner is alive, so no queued work outlives it.
        DrainEvents();
        state->commissioner.Shutdown();
        DeviceControllerFactory::GetInstance().ReleaseSystemState();
        DeviceControllerFactory::GetInstance().Shutdown();
        Credentials::SetGroupDataProvider(nullptr);
        state->groups.Finish();
        state.reset();
        Platform::MemoryShutdown();
    }

    // Runs the work already queued, then returns. The tests that check synchronous behaviour never call this between
    // the action and the check.
    static void DrainEvents()
    {
        ASSERT_EQ(DeviceLayer::PlatformMgr().ScheduleWork(
                      [](intptr_t) { EXPECT_EQ(DeviceLayer::PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

    static Transport::PeerAddress LoopbackPeer(uint16_t port)
    {
        Inet::IPAddress loopback;
        EXPECT_TRUE(Inet::IPAddress::FromString("::1", loopback));
        return Transport::PeerAddress::UDP(loopback, port);
    }

    // Starts PASE establishment through the real EstablishPASEConnection, to a loopback port nobody answers on. The
    // commissionee is left in the PASE state (the proxy and its PASESession from Pair() exist; mDeviceInPASEEstablishment is set).
    CHIP_ERROR StartPASE(NodeId deviceId, uint16_t port)
    {
        RendezvousParameters parameters;
        parameters.SetPeerAddress(LoopbackPeer(port)).SetSetupPINCode(20202021);
        return state->commissioner.EstablishPASEConnection(deviceId, parameters);
    }

    // Takes a commissionee from the PASE state to the Idle state (OnPairingComplete(CHIP_NO_ERROR), no Commission() yet).
    // Everything before PASE completion is the real EstablishPASEConnection; only the completion is a shortcut: a test
    // session is injected and handed to OnSessionEstablished, and the proxy's own PASESession from Pair(), which can never
    // finish, is cleared first.
    CommissioneeDeviceProxy * PairIdle(NodeId deviceId, uint16_t port, uint16_t localSessionId)
    {
        Testing::DeviceCommissionerTestAccess access(&state->commissioner);
        EXPECT_EQ(StartPASE(deviceId, port), CHIP_NO_ERROR);
        auto * device = access.FindCommissioneeDevice(deviceId);
        if (device == nullptr)
        {
            return nullptr;
        }
        device->GetPairing().Clear();

        SessionHolder holder;
        CHIP_ERROR err = access.GetSessionManager()->InjectPaseSessionWithTestKey(
            holder, localSessionId, deviceId, /* peerSessionId */ 2, kUndefinedFabricIndex, LoopbackPeer(port),
            CryptoContext::SessionRole::kInitiator);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        if (err != CHIP_NO_ERROR)
        {
            return nullptr;
        }
        access.CompletePASE(holder.Get().Value());
        // The proxy holds its own reference now.
        holder.Release();
        return device;
    }

    static void ExpireSession(CommissioneeDeviceProxy & device)
    {
        ASSERT_TRUE(device.GetSecureSession().HasValue());
        device.GetSecureSession().Value()->AsSecureSession()->MarkForEviction();
    }

    RecordingPairingDelegate & Delegate() { return state->pairingDelegate; }

    std::unique_ptr<State> state;
};

// 1: the end of the Idle state when the PASE session is released.
TEST_F(TestCommissioneeSessionLoss, ReleasesIdleCommissioneeSynchronouslyWhenPASESessionIsReleased)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);
    ASSERT_EQ(Delegate().pairingCompletes.size(), 1u);

    bool poolHadCommissioneeInCallback = true;
    Delegate().onReleased              = [&](NodeId, CHIP_ERROR) {
        poolHadCommissioneeInCallback = access.FindCommissioneeDevice(kNodeA) != nullptr;
    };

    access.GetSessionManager()->ExpireAllPASESessions();

    // No event loop has run: the release and the callback are synchronous.
    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, kSessionReleasedReason);
    // The commissionee had already been released when the delegate was called.
    EXPECT_FALSE(poolHadCommissioneeInCallback);
    // Not a PASE or commissioning ending.
    EXPECT_EQ(Delegate().pairingCompletes.size(), 1u);
    EXPECT_TRUE(Delegate().commissioningCompletes.empty());

    DrainEvents();
    EXPECT_EQ(Delegate().released.size(), 1u);
}

// 2: StopPairing on an idle commissionee ends the Idle state with CHIP_ERROR_CANCELLED.
TEST_F(TestCommissioneeSessionLoss, StopPairingOnIdleCommissioneeReportsCancelled)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);

    EXPECT_EQ(state->commissioner.StopPairing(kNodeA), CHIP_NO_ERROR);

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(Delegate().pairingCompletes.size(), 1u);
    EXPECT_TRUE(Delegate().commissioningCompletes.empty());
}

// 3: EstablishPASEConnection for another node ID at the address of an idle commissionee replaces it.
TEST_F(TestCommissioneeSessionLoss, EstablishPASEConnectionReplacingIdleCommissioneeReportsIt)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);

    ASSERT_EQ(StartPASE(kNodeC, kPortA), CHIP_NO_ERROR);

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, CHIP_ERROR_CONNECTION_ABORTED);
    // The replacement is in the PASE state, which this callback does not cover.
    auto * replacement = access.FindCommissioneeDevice(kNodeC);
    ASSERT_NE(replacement, nullptr);
    EXPECT_TRUE(replacement->IsSessionSetupInProgress());
    EXPECT_EQ(Delegate().ReleasedCount(kNodeC), 0u);
    EXPECT_EQ(Delegate().pairingCompletes.size(), 1u);
}

// 4: a commissionee being commissioned is neither released nor reported when its session goes away, because
// AutoCommissioner would be left with a dangling proxy.
TEST_F(TestCommissioneeSessionLoss, CommissioneeBeingCommissionedIsNotReleasedWhenItsSessionIsReleased)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    auto * device = PairIdle(kNodeA, kPortA, 0x1001);
    ASSERT_NE(device, nullptr);
    access.SetDeviceBeingCommissioned(device);

    access.GetSessionManager()->ExpireAllPASESessions();
    DrainEvents();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), device);
    EXPECT_TRUE(Delegate().released.empty());
    EXPECT_TRUE(Delegate().commissioningCompletes.empty());

    access.SetDeviceBeingCommissioned(nullptr);
}

// 5: a commissionee in PASE establishment ends with OnPairingComplete, not with this callback.
TEST_F(TestCommissioneeSessionLoss, CommissioneeInPASEEstablishmentIsNotReported)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_EQ(StartPASE(kNodeA, kPortA), CHIP_NO_ERROR);
    ASSERT_NE(access.FindCommissioneeDevice(kNodeA), nullptr);

    // Releases the session the handshake is using, which fails the handshake.
    access.GetSessionManager()->ExpireAllPASESessions();
    DrainEvents();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().pairingCompletes.size(), 1u);
    EXPECT_NE(Delegate().pairingCompletes[0], CHIP_NO_ERROR);
    EXPECT_TRUE(Delegate().released.empty());
}

// 5b: StopPairing in a direct PASE handshake is not an idle release either; only the idle case of its else-branch reports.
TEST_F(TestCommissioneeSessionLoss, StopPairingDuringPASEEstablishmentIsNotReported)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_EQ(StartPASE(kNodeA, kPortA), CHIP_NO_ERROR);

    EXPECT_EQ(state->commissioner.StopPairing(kNodeA), CHIP_NO_ERROR);

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    EXPECT_TRUE(Delegate().released.empty());
}

// 6: a defunct PASE session that is then released ends the Idle state like any other release.
TEST_F(TestCommissioneeSessionLoss, DefunctThenReleasedIsReportedOnce)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    auto * device = PairIdle(kNodeA, kPortA, 0x1001);
    ASSERT_NE(device, nullptr);
    ASSERT_TRUE(device->GetSecureSession().HasValue());

    device->GetSecureSession().Value()->AsSecureSession()->MarkAsDefunct();
    // Defunct is not released.
    EXPECT_NE(access.FindCommissioneeDevice(kNodeA), nullptr);
    EXPECT_TRUE(Delegate().released.empty());

    access.GetSessionManager()->ExpireAllPASESessions();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, kSessionReleasedReason);

    DrainEvents();
    EXPECT_EQ(Delegate().released.size(), 1u);
}

// 7: Shutdown() releases idle commissionees without reporting them.
TEST_F(TestCommissioneeSessionLoss, ShutdownDoesNotReportIdleCommissionee)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);

    state->commissioner.Shutdown();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    EXPECT_TRUE(Delegate().released.empty());
}

// 8: the second idle window. A failure at or after network setup keeps the PASE session for a retry (the real
// CleanupCommissioning decides that); when that session is released later, the callback must still fire.
TEST_F(TestCommissioneeSessionLoss, CommissioneeKeptAfterFailedCommissioningIsReportedWhenSessionIsReleased)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    auto * device = PairIdle(kNodeA, kPortA, 0x1001);
    ASSERT_NE(device, nullptr);

    CompletionStatus failure;
    failure.err = CHIP_ERROR_TIMEOUT;
    failure.failedStage.SetValue(CommissioningStage::kWiFiNetworkSetup);
    access.SetDeviceBeingCommissioned(device);
    access.CleanupCommissioning(device, kNodeA, failure);

    // Commissioning ended with OnCommissioningComplete and kept the commissionee, idle again.
    ASSERT_EQ(Delegate().commissioningCompletes.size(), 1u);
    EXPECT_EQ(Delegate().commissioningCompletes[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().commissioningCompletes[0].error, CHIP_ERROR_TIMEOUT);
    ASSERT_EQ(access.FindCommissioneeDevice(kNodeA), device);
    EXPECT_TRUE(device->IsSecureConnected());
    EXPECT_TRUE(Delegate().released.empty());

    access.GetSessionManager()->ExpireAllPASESessions();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, kSessionReleasedReason);
    EXPECT_EQ(Delegate().commissioningCompletes.size(), 1u);
}

// 8b: CleanupCommissioning releases the proxy after mDeviceBeingCommissioned is cleared; that release is the end of
// commissioning (OnCommissioningComplete), not of the Idle state, so it must not be reported.
TEST_F(TestCommissioneeSessionLoss, SuccessfulCommissioningCleanupDoesNotReportRelease)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    auto * device = PairIdle(kNodeA, kPortA, 0x1001);
    ASSERT_NE(device, nullptr);

    access.SetDeviceBeingCommissioned(device);
    access.CleanupCommissioning(device, kNodeA, CompletionStatus());

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().commissioningCompletes.size(), 1u);
    EXPECT_EQ(Delegate().commissioningCompletes[0].error, CHIP_NO_ERROR);
    EXPECT_TRUE(Delegate().released.empty());
}

// 9: while a setup-code PASE attempt for B is running, SetUpCodePairer is the pairing delegate and has to pass the release
// of idle commissionee A on to the real delegate, without disturbing B's attempt.
TEST_F(TestCommissioneeSessionLoss, SetUpCodePairerForwardsReleaseOfIdleCommissioneeDuringAnotherAttempt)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    auto * deviceA = PairIdle(kNodeA, kPortA, 0x1001);
    ASSERT_NE(deviceA, nullptr);

    // Set up the state a setup-code attempt for B is in (same steps as TestBleFallback::BeginUdp).
    auto & pairer = access.GetSetUpCodePairer();
    Testing::SetUpCodePairerTestAccess pairerAccess(&pairer);
    SetupPayload payload;
    payload.setUpPINCode = 20202021;
    payload.discriminator.SetLongValue(0xABC);
    pairerAccess.SetSetupPayload(payload);
    pairerAccess.SetRemoteId(kNodeB);
    SetUpCodePairerParameters udp;
    udp.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
    pairerAccess.SetCurrentPASEParameters(udp);
    pairerAccess.ExpectPASEEstablishment();
    ASSERT_EQ(state->commissioner.GetPairingDelegate(), &pairer);
    ASSERT_EQ(StartPASE(kNodeB, kPortB), CHIP_NO_ERROR);
    auto * deviceB = access.FindCommissioneeDevice(kNodeB);
    ASSERT_NE(deviceB, nullptr);
    const size_t pairingCompletesBefore = Delegate().pairingCompletes.size();

    // Release only A's session; ExpireAllPASESessions() would abort B's handshake too.
    ExpireSession(*deviceA);

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    ASSERT_EQ(Delegate().released.size(), 1u);
    EXPECT_EQ(Delegate().released[0].deviceId, kNodeA);
    EXPECT_EQ(Delegate().released[0].reason, kSessionReleasedReason);
    // B's attempt is untouched.
    EXPECT_EQ(access.FindCommissioneeDevice(kNodeB), deviceB);
    EXPECT_TRUE(deviceB->IsSessionSetupInProgress());
    EXPECT_TRUE(pairerAccess.GetWaitingForPASE());
    EXPECT_EQ(state->commissioner.GetPairingDelegate(), &pairer);
    EXPECT_EQ(Delegate().pairingCompletes.size(), pairingCompletesBefore);
    EXPECT_TRUE(Delegate().commissioningCompletes.empty());
}

// 10: several idle commissionees released together are each reported once.
TEST_F(TestCommissioneeSessionLoss, TwoIdleCommissioneesReleasedTogetherAreEachReportedOnce)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);
    ASSERT_NE(PairIdle(kNodeB, kPortB, 0x1002), nullptr);

    access.GetSessionManager()->ExpireAllPASESessions();

    EXPECT_EQ(access.FindCommissioneeDevice(kNodeA), nullptr);
    EXPECT_EQ(access.FindCommissioneeDevice(kNodeB), nullptr);
    EXPECT_EQ(Delegate().released.size(), 2u);
    EXPECT_EQ(Delegate().ReleasedCount(kNodeA), 1u);
    EXPECT_EQ(Delegate().ReleasedCount(kNodeB), 1u);

    DrainEvents();
    EXPECT_EQ(Delegate().released.size(), 2u);
}

// 11: once released, an idle commissionee is gone: StopPairing finds nothing and does not report it a second time.
TEST_F(TestCommissioneeSessionLoss, StopPairingAfterReleaseFindsNothingAndReportsNothingMore)
{
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);

    Testing::DeviceCommissionerTestAccess(&state->commissioner).GetSessionManager()->ExpireAllPASESessions();

    EXPECT_EQ(state->commissioner.StopPairing(kNodeA), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    EXPECT_EQ(Delegate().released.size(), 1u);
}

// 12: regression guard. A commissionee whose PASE session is still live stays idle in the pool.
TEST_F(TestCommissioneeSessionLoss, CommissioneeWithLivePASESessionIsKept)
{
    Testing::DeviceCommissionerTestAccess access(&state->commissioner);
    ASSERT_NE(PairIdle(kNodeA, kPortA, 0x1001), nullptr);

    DrainEvents();

    EXPECT_NE(access.FindCommissioneeDevice(kNodeA), nullptr);
    EXPECT_TRUE(Delegate().released.empty());
}

} // namespace
