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

#include <posix/emulated_network/EmulatedWiFiDriver.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer::NetworkCommissioning;

namespace {

ByteSpan Bytes(CharSpan text)
{
    return ByteSpan(Uint8::from_const_char(text.data()), text.size());
}

const ByteSpan kSsid1 = Bytes("network-1"_span);
const ByteSpan kSsid2 = Bytes("network-2"_span);
const ByteSpan kSsid3 = Bytes("network-3"_span);

class ConnectResult : public WiFiDriver::ConnectCallback
{
public:
    void OnResult(Status status, CharSpan /* debugText */, int32_t /* connectStatus */) override { result = status; }

    std::optional<Status> result;
};

class TestEmulatedWiFiDriver : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mDriver.Init(nullptr), CHIP_NO_ERROR); }
    void TearDown() override { mDriver.Shutdown(); }

protected:
    Status Add(ByteSpan ssid)
    {
        char debugTextBuffer[64];
        MutableCharSpan debugText(debugTextBuffer);
        uint8_t index;
        return mDriver.AddOrUpdateNetwork(ssid, Bytes("password"_span), debugText, index);
    }

    std::optional<bool> IsConnected(ByteSpan ssid)
    {
        std::optional<bool> connected;
        NetworkIterator * networks = mDriver.GetNetworks();
        Network network;
        while (networks->Next(network))
        {
            if (ByteSpan(network.networkID, network.networkIDLen).data_equal(ssid))
            {
                connected = network.connected;
            }
        }
        networks->Release();
        return connected;
    }

    EmulatedWiFiDriver mDriver{ EmulatedWiFiDriver::Config{ .maxNetworks = 3 } };
};

TEST_F(TestEmulatedWiFiDriver, ConnectReportsResultSynchronously)
{
    ConnectResult unknown;
    mDriver.ConnectNetwork(kSsid1, &unknown);
    EXPECT_EQ(unknown.result, Status::kNetworkIDNotFound);

    ASSERT_EQ(Add(kSsid1), Status::kSuccess);
    ASSERT_EQ(Add(kSsid2), Status::kSuccess);
    EXPECT_EQ(IsConnected(kSsid1), false);

    ConnectResult known;
    mDriver.ConnectNetwork(kSsid1, &known);
    EXPECT_EQ(known.result, Status::kSuccess);
    EXPECT_EQ(IsConnected(kSsid1), true);
    EXPECT_EQ(IsConnected(kSsid2), false);
}

TEST_F(TestEmulatedWiFiDriver, RevertRestoresCommittedNetworksAndConnection)
{
    ASSERT_EQ(Add(kSsid1), Status::kSuccess);
    ASSERT_EQ(mDriver.CommitConfiguration(), CHIP_NO_ERROR);

    ASSERT_EQ(Add(kSsid2), Status::kSuccess);
    ConnectResult result;
    mDriver.ConnectNetwork(kSsid2, &result);
    ASSERT_EQ(result.result, Status::kSuccess);

    // The connected network was never committed, so reverting also drops the connection.
    ASSERT_EQ(mDriver.RevertConfiguration(), CHIP_NO_ERROR);
    EXPECT_EQ(IsConnected(kSsid1), false);
    EXPECT_EQ(IsConnected(kSsid2), std::nullopt);

    mDriver.ConnectNetwork(kSsid1, &result);
    ASSERT_EQ(result.result, Status::kSuccess);
    ASSERT_EQ(mDriver.RevertConfiguration(), CHIP_NO_ERROR);
    EXPECT_EQ(IsConnected(kSsid1), true);
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
TEST_F(TestEmulatedWiFiDriver, PDCClientIdentityCanBeReused)
{
    // Any well-formed compact identity will do for the network identity.
    uint8_t networkIdentityBuffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan networkIdentity(networkIdentityBuffer);
    {
        Crypto::P256Keypair keypair;
        ASSERT_EQ(keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);
        ASSERT_EQ(Credentials::NewChipNetworkIdentity(keypair, networkIdentity), CHIP_NO_ERROR);
    }

    auto addWithPDC = [&](ByteSpan ssid, Optional<uint8_t> reuseIndex, MutableByteSpan & clientIdentity, uint8_t & index) {
        Status status;
        char debugTextBuffer[64];
        MutableCharSpan debugText(debugTextBuffer);
        EXPECT_EQ(mDriver.AddOrUpdateNetworkWithPDC(ssid, networkIdentity, reuseIndex, status, debugText, clientIdentity, index),
                  CHIP_NO_ERROR);
        return status;
    };

    uint8_t identity1Buffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    uint8_t identity2Buffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    uint8_t identity3Buffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity1(identity1Buffer);
    MutableByteSpan identity2(identity2Buffer);
    MutableByteSpan identity3(identity3Buffer);
    uint8_t index1, index2, index3;

    ASSERT_EQ(addWithPDC(kSsid1, NullOptional, identity1, index1), Status::kSuccess);
    ASSERT_EQ(addWithPDC(kSsid2, MakeOptional(index1), identity2, index2), Status::kSuccess);
    EXPECT_TRUE(identity2.data_equal(identity1));

    // Updating a network without reuse generates a new client identity.
    ASSERT_EQ(addWithPDC(kSsid2, NullOptional, identity3, index3), Status::kSuccess);
    EXPECT_EQ(index3, index2);
    EXPECT_FALSE(identity3.data_equal(identity1));

    // A reused identity must refer to an existing PDC network.
    MutableByteSpan unused(identity3Buffer);
    EXPECT_EQ(addWithPDC(kSsid3, MakeOptional(static_cast<uint8_t>(5)), unused, index3), Status::kOutOfRange);
    ASSERT_EQ(Add(kSsid1), Status::kSuccess); // replaces PDC with a passphrase
    EXPECT_EQ(addWithPDC(kSsid3, MakeOptional(index1), unused, index3), Status::kOutOfRange);

    uint8_t clientIdentityBuffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan clientIdentity(clientIdentityBuffer);
    EXPECT_EQ(mDriver.GetClientIdentity(index1, clientIdentity), CHIP_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(mDriver.GetClientIdentity(index2, clientIdentity), CHIP_NO_ERROR);
    EXPECT_TRUE(clientIdentity.data_equal(identity3));
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

} // namespace
