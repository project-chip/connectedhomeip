/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/SetUpCodePairer.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <controller/tests/SetUpCodePairerTestAccess.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/RawKeySessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <transport/raw/PeerAddress.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif

using namespace chip;
using namespace chip::Controller;
using PairerAccess = chip::Testing::SetUpCodePairerTestAccess;

namespace {

#if CONFIG_NETWORK_LAYER_BLE
class PairerBleLayer : public Ble::BleLayer,
                       private Ble::BlePlatformDelegate,
                       private Ble::BleApplicationDelegate,
                       public Ble::BleConnectionDelegate
{
public:
    CHIP_ERROR Initialize(System::Layer & systemLayer) { return Init(this, this, &systemLayer); }
    CHIP_ERROR Install(Ble::BleLayer & layer) { return layer.Init(this, this, this, &DeviceLayer::SystemLayer()); }

    unsigned closeCount                    = 0;
    BLE_CONNECTION_OBJECT closedConnection = BLE_CONNECTION_UNINITIALIZED;
    unsigned requestCount                  = 0;
    unsigned cancelCount                   = 0;
    SetupDiscriminator requestedDiscriminator;
    void * backendContext = nullptr;

    void NewConnection(Ble::BleLayer *, void * context, const SetupDiscriminator & discriminator) override
    {
        ++requestCount;
        requestedDiscriminator = discriminator;
        backendContext         = context;
    }
    void NewConnection(Ble::BleLayer *, void *, BLE_CONNECTION_OBJECT) override { FAIL(); }
    CHIP_ERROR CancelConnection() override
    {
        ++cancelCount;
        return CHIP_NO_ERROR;
    }

private:
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT connection) override
    {
        ++closeCount;
        closedConnection = connection;
        return CHIP_NO_ERROR;
    }
    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 0; }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                              System::PacketBufferHandle) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                                System::PacketBufferHandle) override
    {
        return CHIP_NO_ERROR;
    }
};
#endif

template <typename T>
static typename std::enable_if<std::is_integral<T>::value, T>::type MakeTestConnectionObject(uint8_t *)
{
    return static_cast<T>(1);
}

template <typename T>
static typename std::enable_if<std::is_pointer<T>::value, T>::type MakeTestConnectionObject(uint8_t * storage)
{
    return reinterpret_cast<T>(storage);
}

constexpr uint16_t kDiscriminatorA = 0xABC;
constexpr uint16_t kDiscriminatorB = 0xDEF;

#if CONFIG_NETWORK_LAYER_BLE && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
class PaseOnlyCredentials : public OperationalCredentialsDelegate
{
    CHIP_ERROR GenerateNOCChain(const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &,
                                const ByteSpan &, Callback::Callback<OnNOCChainGeneration> *) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

class TestBleCommissioner : public ::testing::Test
{
public:
    struct State
    {
        TestPersistentStorageDelegate storage;
        Credentials::PersistentStorageOpCertStore certificates;
        Crypto::RawKeySessionKeystore keys;
        Credentials::GroupDataProviderImpl groups;
        PaseOnlyCredentials credentials;
        PairerBleLayer ble;
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
        FactoryInitParams factoryParams;
        factoryParams.fabricIndependentStorage = &state->storage;
        factoryParams.opCertStore              = &state->certificates;
        factoryParams.sessionKeystore          = &state->keys;
        factoryParams.groupDataProvider        = &state->groups;
        factoryParams.dataModelProvider        = app::CodegenDataModelProviderInstance(&state->storage);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().Init(factoryParams), CHIP_NO_ERROR);
        DeviceControllerFactory::GetInstance().RetainSystemState();
        auto * layer = DeviceControllerFactory::GetInstance().GetSystemState()->BleLayer();
        layer->Shutdown();
        ASSERT_EQ(state->ble.Install(*layer), CHIP_NO_ERROR);
        SetupParams params;
        params.controllerVendorId             = VendorId::TestVendor1;
        params.operationalCredentialsDelegate = &state->credentials;
        params.deviceAttestationVerifier      = Credentials::GetDefaultDACVerifier(nullptr);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(params, state->commissioner), CHIP_NO_ERROR);
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

    RendezvousParameters ReconnectParameters()
    {
        RendezvousParameters params;
        SetupDiscriminator discriminator;
        discriminator.SetLongValue(kDiscriminatorA);
        params.SetPeerAddress(Transport::PeerAddress::BLE()).SetSetupPINCode(20202021).SetSetupDiscriminator(discriminator);
        return params;
    }

protected:
    std::unique_ptr<State> state;
};

TEST_F(TestBleCommissioner, StopPairingCancelsReconnectAndClosesLateSuccess)
{
    auto params = ReconnectParameters();
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.requestCount, 1u);
    EXPECT_EQ(state->ble.requestedDiscriminator.GetLongValue(), kDiscriminatorA);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancelCount, 1u);
    uint8_t connectionStorage = 0;
    auto connection           = MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionStorage);
    state->ble.Ble::BleConnectionDelegate::OnConnectionComplete(state->ble.backendContext, connection);
    EXPECT_EQ(state->ble.closeCount, 1u);
    EXPECT_EQ(state->ble.closedConnection, connection);
    EXPECT_EQ(state->ble.requestCount, 1u);
}

TEST_F(TestBleCommissioner, ShutdownDetachesPendingReconnect)
{
    auto params = ReconnectParameters();
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    state->commissioner.Shutdown();
    EXPECT_EQ(state->ble.cancelCount, 1u);
    state->ble.Ble::BleConnectionDelegate::OnConnectionError(state->ble.backendContext, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(state->ble.requestCount, 1u);
}

TEST_F(TestBleCommissioner, DestroyedCommissionerCannotReceiveLateCallbacks)
{
    auto commissioner = std::make_unique<DeviceCommissioner>();
    SetupParams setup;
    setup.controllerVendorId             = VendorId::TestVendor1;
    setup.operationalCredentialsDelegate = &state->credentials;
    setup.deviceAttestationVerifier      = Credentials::GetDefaultDACVerifier(nullptr);
    ASSERT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(setup, *commissioner), CHIP_NO_ERROR);
    auto params = ReconnectParameters();
    ASSERT_EQ(commissioner->EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    commissioner->Shutdown();
    commissioner.reset();
    EXPECT_EQ(state->ble.cancelCount, 1u);
    uint8_t connectionStorage = 0;
    auto connection           = MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionStorage);
    state->ble.Ble::BleConnectionDelegate::OnConnectionComplete(state->ble.backendContext, connection);
    state->ble.Ble::BleConnectionDelegate::OnConnectionError(state->ble.backendContext, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(state->ble.closeCount, 1u);
    EXPECT_EQ(state->ble.closedConnection, connection);
}

TEST_F(TestBleCommissioner, NewPairingAfterCancellation)
{
    auto params = ReconnectParameters();
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(2, params), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.requestCount, 2u);
    ASSERT_EQ(state->commissioner.StopPairing(2), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancelCount, 2u);
}

TEST_F(TestBleCommissioner, DeferredDiscoveryPaseFailureStartsFreshRequest)
{
    auto & pairer = Testing::DeviceCommissionerTestAccess(&state->commissioner).GetSetUpCodePairer();
    PairerAccess access(&pairer);
    SetupPayload payload;
    payload.setUpPINCode = 20202021;
    payload.discriminator.SetLongValue(kDiscriminatorA);
    access.SetSetupPayload(payload);
    access.SetRemoteId(1);
    access.ExpectPASEEstablishment();
    uint8_t connectionStorage = 0;
    auto discarded            = MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionStorage);
    access.CallOnDiscoveredDeviceOverBle(discarded, kDiscriminatorA);
    ASSERT_EQ(state->ble.closeCount, 1u);
    EXPECT_EQ(state->ble.closedConnection, discarded);
    ASSERT_EQ(access.GetDiscoveredParametersCount(), 1u);
    EXPECT_FALSE(access.FrontDiscoveredParameters().HasConnectionObject());
    access.CallOnPairingComplete(CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(state->ble.requestCount, 1u);
    EXPECT_EQ(state->ble.requestedDiscriminator.GetLongValue(), kDiscriminatorA);
    EXPECT_EQ(state->ble.closeCount, 1u);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancelCount, 1u);
}

TEST_F(TestBleCommissioner, ImmediateBleConnectionIsNotDiscarded)
{
    auto params               = ReconnectParameters();
    uint8_t connectionStorage = 0;
    params.SetConnectionObject(MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionStorage));
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.requestCount, 0u);
    EXPECT_EQ(state->ble.closeCount, 0u);
    EXPECT_EQ(state->ble.cancelCount, 0u);
    EXPECT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
}

TEST_F(TestBleCommissioner, ReconnectSuccessConsumesConnectionWithoutCancellation)
{
    auto params = ReconnectParameters();
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    uint8_t connectionStorage = 0;
    auto connection           = MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionStorage);
    state->ble.Ble::BleConnectionDelegate::OnConnectionComplete(state->ble.backendContext, connection);
    EXPECT_EQ(state->ble.requestCount, 1u);
    EXPECT_EQ(state->ble.cancelCount, 0u);
    EXPECT_EQ(state->ble.closeCount, 0u);
    EXPECT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancelCount, 0u);
}
#endif

Inet::IPAddress ParseIP(const char * str)
{
    Inet::IPAddress addr;
    // The callers pass valid literals; treat a parse failure as a test bug.
    VerifyOrDie(Inet::IPAddress::FromString(str, addr));
    return addr;
}

// InterfaceId::PlatformType is an integer on the host builds these tests run on; cast explicitly so
// the suite's -Wconversion stays clean regardless of the exact underlying type.
Inet::InterfaceId MakeInterfaceId(unsigned int id)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    static struct netif sNetifs[2] = {};
    VerifyOrDie(id >= 1 && id <= MATTER_ARRAY_SIZE(sNetifs));
    return Inet::InterfaceId(&sNetifs[id - 1]);
#else
    return Inet::InterfaceId(static_cast<Inet::InterfaceId::PlatformType>(id));
#endif
}

// Build a single-address commissionable-node resolution record for the given address and interface.
Dnssd::CommonResolutionData MakeResolutionData(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId)
{
    Dnssd::CommonResolutionData data;
    data.interfaceId  = interfaceId;
    data.numIPs       = 1;
    data.ipAddress[0] = ip;
    data.port         = 5540;
    return data;
}

// DeviceCommissioner is too large to embed in a test fixture (it exceeds
// the pw_unit_test light backend's static memory pool).  Heap-allocate it
// along with the SetUpCodePairer and test accessor that depend on it.
class TestSetUpCodePairer : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mCommissioner = std::make_unique<DeviceCommissioner>();
        mPairer       = std::make_unique<SetUpCodePairer>(mCommissioner.get());
        mAccess       = std::make_unique<PairerAccess>(mPairer.get());
    }

    void TearDown() override
    {
        mAccess.reset();
        mPairer.reset();
        mCommissioner.reset();
    }

protected:
    PairerAccess & Access() { return *mAccess; }

private:
    std::unique_ptr<DeviceCommissioner> mCommissioner;
    std::unique_ptr<SetUpCodePairer> mPairer;
    std::unique_ptr<PairerAccess> mAccess;
};

// When the discovery timeout fires while a PASE attempt is in progress,
// DNS-SD should be stopped (it runs indefinitely) but other transports
// (BLE, Wi-Fi PAF, NFC) should be left alone since they self-terminate.
TEST_F(TestSetUpCodePairer, TimeoutDuringPASE_StopsDNSSD_PreservesOtherTransports)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForPASE(true);
    Access().SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    Access().SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    Access().FireTimeoutCallback();

    // DNS-SD must be stopped to prevent DiscoveryInProgress() from being stuck true.
    EXPECT_FALSE(Access().GetWaitingForDiscovery(PairerAccess::kIPTransport));
    // BLE must be preserved — it may still discover a commissionee.
    EXPECT_TRUE(Access().GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // PASE state must not be disturbed.
    EXPECT_TRUE(Access().GetWaitingForPASE());
}

// When the discovery timeout fires with no PASE in progress,
// all transports should be stopped and failure reported.
TEST_F(TestSetUpCodePairer, TimeoutNoPASE_StopsAllTransports)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForPASE(false);
    Access().SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    Access().SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    Access().FireTimeoutCallback();

    EXPECT_FALSE(Access().GetWaitingForDiscovery(PairerAccess::kIPTransport));
    EXPECT_FALSE(Access().GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // StopPairingIfTransportsExhausted should have reported failure and cleared mRemoteId.
    EXPECT_EQ(Access().GetRemoteId(), kUndefinedNodeId);
}

// When a PASE attempt fails (OnPairingComplete with error) and DNS-SD
// is still running, DNS-SD should NOT be stopped.  Keeping DNS-SD alive
// keeps DiscoveryInProgress() true, which suppresses premature failure
// through OnStatusUpdate, TryNextRendezvousParameters, and
// StopPairingIfTransportsExhausted.
TEST_F(TestSetUpCodePairer, PASEFailure_DNSSDStaysAlive)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    Access().SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    // Simulate a UDP PASE attempt: set mCurrentPASEParameters and enter
    // the PASE-waiting state (as ConnectToDiscoveredDevice would).
    SetUpCodePairerParameters udpParams;
    udpParams.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
    Access().SetCurrentPASEParameters(udpParams);
    Access().ExpectPASEEstablishment();

    // PASE fails.
    Access().CallOnPairingComplete(CHIP_ERROR_TIMEOUT);

    // DNS-SD must still be running — the 30s timeout is its natural upper bound.
    EXPECT_TRUE(Access().GetWaitingForDiscovery(PairerAccess::kIPTransport));
    // BLE must still be running.
    EXPECT_TRUE(Access().GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // The pairer should still be active (not yet reported failure).
    EXPECT_NE(Access().GetRemoteId(), kUndefinedNodeId);
    // The error should be saved for later.
    EXPECT_EQ(Access().GetLastPASEError(), CHIP_ERROR_TIMEOUT);
    // mCurrentPASEParameters should have been cleared.
    EXPECT_FALSE(Access().HasCurrentPASEParameters());
}

// When a PASE attempt fails synchronously in ConnectToDiscoveredDevice,
// mCurrentPASEParameters must be cleared so a later non-UDP PASE
// completion cannot incorrectly use stale parameters for ReconfirmRecord.
TEST_F(TestSetUpCodePairer, SyncPASEFailure_ClearsCurrentPASEParameters)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForDiscovery(PairerAccess::kIPTransport, true);

    // Set up stale UDP parameters (as if ConnectToDiscoveredDevice had set
    // them before calling PairDevice, which then failed synchronously).
    SetUpCodePairerParameters udpParams;
    udpParams.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
    Access().SetCurrentPASEParameters(udpParams);

    // Now simulate a subsequent non-UDP PASE completion (e.g., BLE PASE
    // succeeds or fails).  ExpectPASEEstablishment + OnPairingComplete
    // with success exercises ResetDiscoveryState which clears everything.
    // Instead, verify directly that after a PASE failure the stale
    // parameters don't persist: simulate a PASE completion with error.
    Access().ExpectPASEEstablishment();
    Access().CallOnPairingComplete(CHIP_ERROR_CONNECTION_ABORTED);

    // mCurrentPASEParameters must be cleared — stale UDP params must not
    // survive to confuse a later ReconfirmRecord check.
    EXPECT_FALSE(Access().HasCurrentPASEParameters());
}

#if CONFIG_NETWORK_LAYER_BLE
TEST_F(TestSetUpCodePairer, DeferredBleDiscoveryQueuesReconnectableParams)
{
    auto & systemLayer = DeviceLayer::SystemLayer();
    ASSERT_EQ(systemLayer.Init(), CHIP_NO_ERROR);
    PairerBleLayer bleLayer;
    ASSERT_EQ(bleLayer.Initialize(systemLayer), CHIP_NO_ERROR);
    Access().SetBleLayer(&bleLayer);
    Access().SetWaitingForPASE(true);

    uint8_t connectionObjectStorage;
    BLE_CONNECTION_OBJECT connObj = MakeTestConnectionObject<BLE_CONNECTION_OBJECT>(&connectionObjectStorage);
    Access().CallOnDiscoveredDeviceOverBle(connObj, std::optional<uint16_t>{ 3840 });

    ASSERT_EQ(Access().GetDiscoveredParametersCount(), 1u);

    const auto & params = Access().FrontDiscoveredParameters();
    EXPECT_EQ(params.GetPeerAddress().GetTransportType(), Transport::Type::kBle);
    EXPECT_FALSE(params.HasConnectionObject());
    EXPECT_FALSE(params.HasDiscoveredObject());
    ASSERT_TRUE(params.mLongDiscriminator.has_value());
    EXPECT_EQ(params.mLongDiscriminator, std::optional<uint16_t>{ 3840 });
    EXPECT_EQ(bleLayer.closeCount, 1u);
    EXPECT_EQ(bleLayer.closedConnection, connObj);
    Access().SetBleLayer(nullptr);
    bleLayer.Shutdown();
    systemLayer.Shutdown();
}
#endif

// A non-link-local address advertised on two different interfaces resolves to the same
// interface-less PeerAddress (we rely on the routing table, not the discovery interface), so the
// two candidates must be treated as coalescible.
TEST_F(TestSetUpCodePairer, CanCoalesce_NonLinkLocalSameAddressDifferentInterfaces)
{
    const Inet::IPAddress global = ParseIP("2001:db8::1");
    SetUpCodePairerParameters a(MakeResolutionData(global, MakeInterfaceId(1)), kDiscriminatorA, 0);
    SetUpCodePairerParameters b(MakeResolutionData(global, MakeInterfaceId(2)), kDiscriminatorA, 0);
    EXPECT_TRUE(a.CanCoalesceWith(b));
}

// A link-local address keeps its interface in the PeerAddress, so the same address on two
// interfaces is two genuinely distinct destinations and must not coalesce.
TEST_F(TestSetUpCodePairer, CannotCoalesce_LinkLocalSameAddressDifferentInterfaces)
{
    const Inet::IPAddress lla = ParseIP("fe80::1");
    SetUpCodePairerParameters a(MakeResolutionData(lla, MakeInterfaceId(1)), kDiscriminatorA, 0);
    SetUpCodePairerParameters b(MakeResolutionData(lla, MakeInterfaceId(2)), kDiscriminatorA, 0);
    EXPECT_FALSE(a.CanCoalesceWith(b));
}

// The long discriminator selects which setup payload's passcode we use, so candidates that share a
// PeerAddress but carry different discriminators must not coalesce.
TEST_F(TestSetUpCodePairer, CannotCoalesce_DifferentLongDiscriminator)
{
    const Inet::IPAddress global = ParseIP("2001:db8::1");
    SetUpCodePairerParameters a(MakeResolutionData(global, Inet::InterfaceId::Null()), kDiscriminatorA, 0);
    SetUpCodePairerParameters b(MakeResolutionData(global, Inet::InterfaceId::Null()), kDiscriminatorB, 0);
    EXPECT_FALSE(a.CanCoalesceWith(b));
}

// MRP configuration is a PASE connection input; candidates that differ only in their MRP intervals
// must not coalesce.
TEST_F(TestSetUpCodePairer, CannotCoalesce_DifferentMRPConfig)
{
    const Inet::IPAddress global = ParseIP("2001:db8::1");
    auto dataA                   = MakeResolutionData(global, Inet::InterfaceId::Null());
    dataA.mrpRetryIntervalIdle   = System::Clock::Milliseconds32(1000);
    auto dataB                   = MakeResolutionData(global, Inet::InterfaceId::Null());
    dataB.mrpRetryIntervalIdle   = System::Clock::Milliseconds32(2000);
    SetUpCodePairerParameters a(dataA, kDiscriminatorA, 0);
    SetUpCodePairerParameters b(dataB, kDiscriminatorA, 0);
    EXPECT_FALSE(a.CanCoalesceWith(b));
}

// Different IP addresses are different destinations and must not coalesce.
TEST_F(TestSetUpCodePairer, CannotCoalesce_DifferentAddress)
{
    SetUpCodePairerParameters a(MakeResolutionData(ParseIP("2001:db8::1"), Inet::InterfaceId::Null()), kDiscriminatorA, 0);
    SetUpCodePairerParameters b(MakeResolutionData(ParseIP("2001:db8::2"), Inet::InterfaceId::Null()), kDiscriminatorA, 0);
    EXPECT_FALSE(a.CanCoalesceWith(b));
}

// NotifyCommissionableDeviceDiscovered must drop a duplicate address within a single advertisement
// (the same non-link-local IP listed twice) instead of queuing a redundant PASE attempt.  Waiting
// for PASE is asserted so ConnectToDiscoveredDevice does not drain the queue before we inspect it.
TEST_F(TestSetUpCodePairer, NotifyDiscovered_DropsDuplicateAddressesInOneRecord)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForPASE(true);

    const Inet::IPAddress global = ParseIP("2001:db8::1");
    auto data                    = MakeResolutionData(global, Inet::InterfaceId::Null());
    data.numIPs                  = 2;
    data.ipAddress[0]            = global;
    data.ipAddress[1]            = global; // same address advertised twice

    Access().NotifyCommissionableDeviceDiscovered(data, kDiscriminatorA);
    EXPECT_EQ(Access().GetDiscoveredParametersCount(), 1u);
}

// The same non-link-local device rediscovered on a second interface must not add a second identical
// queue entry, but a genuinely different address must.
TEST_F(TestSetUpCodePairer, NotifyDiscovered_CoalescesAcrossCallbacks)
{
    Access().SetRemoteId(1);
    Access().SetWaitingForPASE(true);

    const Inet::IPAddress global = ParseIP("2001:db8::1");
    Access().NotifyCommissionableDeviceDiscovered(MakeResolutionData(global, MakeInterfaceId(1)), kDiscriminatorA);
    EXPECT_EQ(Access().GetDiscoveredParametersCount(), 1u);

    // Same non-link-local address, different interface -> coalesces.
    Access().NotifyCommissionableDeviceDiscovered(MakeResolutionData(global, MakeInterfaceId(2)), kDiscriminatorA);
    EXPECT_EQ(Access().GetDiscoveredParametersCount(), 1u);

    // Different address -> new entry.
    Access().NotifyCommissionableDeviceDiscovered(MakeResolutionData(ParseIP("2001:db8::2"), MakeInterfaceId(1)), kDiscriminatorA);
    EXPECT_EQ(Access().GetDiscoveredParametersCount(), 2u);
}

} // namespace
