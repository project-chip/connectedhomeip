/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/TestOnlyLocalCertificateAuthority.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>

#include <cstring>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Transport;

namespace {

class CommissioneeDeviceProxyTest : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    CommissioneeDeviceProxy commissioneeDeviceProxy{};
};

TEST_F(CommissioneeDeviceProxyTest, SuccessfullyGetsInitialized)
{
    ControllerDeviceInitParams params;

    params.sessionManager = chip::Platform::New<chip::SessionManager>();
    params.exchangeMgr    = chip::Platform::New<chip::Messaging::ExchangeManager>();
    NodeId deviceId       = 12345;
    char ipAddress[10]    = "127.0.0.1";

    Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(ipAddress, 5540);
    commissioneeDeviceProxy.Init(params, deviceId, peerAddress);

    EXPECT_EQ(commissioneeDeviceProxy.GetPeerId().GetNodeId(), deviceId);
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerAddress(), peerAddress);
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerAddress().GetPort(), 5540);
    EXPECT_TRUE(commissioneeDeviceProxy.IsSessionSetupInProgress());
    EXPECT_FALSE(commissioneeDeviceProxy.IsSecureConnected());
    EXPECT_EQ(commissioneeDeviceProxy.GetDeviceId(), deviceId);
    EXPECT_EQ(commissioneeDeviceProxy.GetDeviceTransportType(), Transport::Type::kUdp);

    chip::Platform::Delete(params.sessionManager);
    chip::Platform::Delete(params.exchangeMgr);
}

TEST_F(CommissioneeDeviceProxyTest, UpdatesPeerDeviceData)
{
    ControllerDeviceInitParams params;

    params.sessionManager = chip::Platform::New<chip::SessionManager>();
    params.exchangeMgr    = chip::Platform::New<chip::Messaging::ExchangeManager>();
    NodeId deviceId       = 12345;
    char ipAddress[10]    = "127.0.0.1";

    Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(ipAddress, 5540);
    commissioneeDeviceProxy.Init(params, deviceId, peerAddress);

    char newIpAddress[39]                 = "2001:db8:1234:5678:90ab:cdef:1234:5678";
    Transport::PeerAddress newPeerAddress = Transport::PeerAddress::UDP(newIpAddress, 5540);
    CHIP_ERROR err                        = commissioneeDeviceProxy.UpdateDeviceData(newPeerAddress,
                                                                                     ReliableMessageProtocolConfig{
                                                                  System::Clock::Milliseconds32(1000), // idle interval
                                                                  System::Clock::Milliseconds32(2000), // active interval
                                                                  System::Clock::Milliseconds16(500) // active threshold
                                                              });

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerAddress(), newPeerAddress);
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerAddress().GetPort(), 5540);
}

TEST_F(CommissioneeDeviceProxyTest, SetsPeerIdFromCertificates)
{
    ControllerDeviceInitParams params;

    params.sessionManager = chip::Platform::New<chip::SessionManager>();
    params.exchangeMgr    = chip::Platform::New<chip::Messaging::ExchangeManager>();
    NodeId deviceId       = 12345;
    char ipAddress[10]    = "127.0.0.1";

    Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(ipAddress, 5540);
    commissioneeDeviceProxy.Init(params, deviceId, peerAddress);

    Credentials::TestOnlyLocalCertificateAuthority fabric11CertAuthority;

    EXPECT_TRUE(fabric11CertAuthority.Init().IsSuccess());

    chip::Crypto::P256Keypair fabric11Node55Keypair; // Fabric ID 11,
    EXPECT_EQ(fabric11Node55Keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    FabricId fabricId = 11;
    NodeId nodeId     = 55;
    EXPECT_EQ(
        fabric11CertAuthority.SetIncludeIcac(false).GenerateNocChain(fabricId, nodeId, fabric11Node55Keypair.Pubkey()).GetStatus(),
        CHIP_NO_ERROR);
    ByteSpan rcac = fabric11CertAuthority.GetRcac();
    ByteSpan noc  = fabric11CertAuthority.GetNoc();

    CHIP_ERROR err = commissioneeDeviceProxy.SetPeerId(rcac, noc);

    NodeId extractedNodeId;
    CompressedFabricId extractedCompressedFabricId;
    Credentials::ExtractNodeIdCompressedFabricIdFromOpCerts(rcac, noc, extractedCompressedFabricId, extractedNodeId);

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerId().GetNodeId(), NodeId{ 55 });
    EXPECT_EQ(commissioneeDeviceProxy.GetPeerId().GetCompressedFabricId(), extractedCompressedFabricId);
}

} // namespace
