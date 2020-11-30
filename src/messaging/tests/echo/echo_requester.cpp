/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a chip-echo-requester, for the
 *      CHIP Echo Protocol.
 *
 *      The CHIP Echo Protocol implements two simple methods, in the
 *      style of ICMP ECHO REQUEST and ECHO REPLY, in which a sent
 *      payload is turned around by the responder and echoed back to
 *      the originator.
 *
 */

#include "common.h"

#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#define ECHO_CLIENT_PORT (CHIP_PORT + 1)

namespace {

// Max value for the number of EchoRequests sent.
constexpr size_t kMaxEchoCount = 3;

// The CHIP Echo interval time in milliseconds.
constexpr int32_t gEchoInterval = 1000;

// The EchoClient object.
chip::Protocols::EchoClient gEchoClient;

chip::TransportMgr<chip::Transport::UDP> gUDPManager;
chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
chip::SecureSessionMgr gSessionManager;
chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Echo was attempted to be sent.
uint64_t gLastEchoTime = 0;

// True, if the EchoClient is waiting for an EchoResponse
// after sending an EchoRequest, false otherwise.
bool gWaitingForEchoResp = false;

// Count of the number of EchoRequests sent.
uint64_t gEchoCount = 0;

// Count of the number of EchoResponses received.
uint64_t gEchoRespCount = 0;

bool gUseTCP = false;

bool EchoIntervalExpired(void)
{
    uint64_t now = chip::System::Timer::GetCurrentEpoch();

    return (now >= gLastEchoTime + gEchoInterval);
}

CHIP_ERROR SendEchoRequest(void)
{
    CHIP_ERROR err                              = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle payloadBuf = chip::System::PacketBuffer::New();

    if (payloadBuf.IsNull())
    {
        printf("Unable to allocate PacketBuffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }
    else
    {
        // Add some application payload data in the buffer.
        char * p    = reinterpret_cast<char *>(payloadBuf->Start());
        int32_t len = snprintf(p, CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE, "Echo Message %" PRIu64 "\n", gEchoCount);

        // Set the datalength in the buffer appropriately.
        payloadBuf->SetDataLength(static_cast<uint16_t>(len));
    }

    gLastEchoTime = chip::System::Timer::GetCurrentEpoch();

    printf("\nSend echo request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = gEchoClient.SendEchoRequest(std::move(payloadBuf));

    if (err == CHIP_NO_ERROR)
    {
        gWaitingForEchoResp = true;
        gEchoCount++;
    }
    else
    {
        printf("Send echo request failed, err: %s\n", chip::ErrorStr(err));
    }

    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Optional<chip::Transport::PeerAddress> peerAddr;
    chip::SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<chip::SecurePairingUsingTestSecret>(
        chip::Optional<chip::NodeId>::Value(chip::kTestDeviceNodeId), static_cast<uint16_t>(0), static_cast<uint16_t>(0));
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (gUseTCP)
    {
        peerAddr = chip::Optional<chip::Transport::PeerAddress>::Value(chip::Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT));
    }
    else
    {
        peerAddr = chip::Optional<chip::Transport::PeerAddress>::Value(
            chip::Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID));
    }

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(peerAddr, chip::kTestDeviceNodeId, testSecurePairingSecret);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastEchoTime = chip::System::Timer::GetCurrentEpoch();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void HandleEchoResponseReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle payload)
{
    uint32_t respTime    = chip::System::Timer::GetCurrentEpoch();
    uint32_t transitTime = respTime - gLastEchoTime;

    gWaitingForEchoResp = false;
    gEchoRespCount++;

    printf("Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", gEchoRespCount, gEchoCount,
           static_cast<double>(gEchoRespCount) * 100 / gEchoCount, payload->DataLength(), static_cast<double>(transitTime) / 1000);
}

class TestSecureSessionMgrDelegate : public chip::SecureSessionMgrDelegate
{
public:
    void OnNewConnection(chip::SecureSessionHandle session, chip::SecureSessionMgr * mgr) override { mSecureSession = session; }

    chip::SecureSessionHandle mSecureSession;
} gTestSecureSessionMgrDelegate;

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (argc <= 1)
    {
        printf("Missing Echo Server IP address\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (argc > 3)
    {
        printf("Too many arguments specified!\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if ((argc == 3) && (strcmp(argv[2], "--tcp") == 0))
    {
        gUseTCP = true;
    }

    if (!chip::Inet::IPAddress::FromString(argv[1], gDestAddr))
    {
        printf("Invalid Echo Server IP address: %s\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    InitializeChip();

    if (gUseTCP)
    {
        err = gTCPManager.Init(chip::Transport::TcpListenParameters(&chip::DeviceLayer::InetLayer)
                                   .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gTCPManager);
        SuccessOrExit(err);
    }
    else
    {
        err = gUDPManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
                                   .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gUDPManager);
        SuccessOrExit(err);
    }

    gSessionManager.SetDelegate(&gTestSecureSessionMgrDelegate);

    err = gExchangeManager.Init(chip::kTestControllerNodeId, &gUDPManager, &gSessionManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    err = gEchoClient.Init(&gExchangeManager, gTestSecureSessionMgrDelegate.mSecureSession);
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    // Connection has been established. Now send the EchoRequests.
    for (unsigned int i = 0; i < kMaxEchoCount; i++)
    {
        if (SendEchoRequest() != CHIP_NO_ERROR)
        {
            printf("Send request failed: %s\n", chip::ErrorStr(err));
            break;
        }

        // Wait for response until the Echo interval.
        while (!EchoIntervalExpired())
        {
            DriveIO();
        }

        // Check if expected response was received.
        if (gWaitingForEchoResp)
        {
            printf("No response received\n");
            gWaitingForEchoResp = false;
        }
    }

    gEchoClient.Shutdown();

    ShutdownChip();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("ChipEchoClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
