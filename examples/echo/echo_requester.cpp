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

#include <protocols/echo/Echo.h>
#include <support/ErrorStr.h>

#define ECHO_CLIENT_PORT (CHIP_PORT + 1)

namespace {

// Max value for the number of EchoRequests sent.
constexpr size_t kMaxEchoCount = 3;

// The CHIP Echo interval time in microseconds.
constexpr int32_t gEchoInterval = 1000000;

// The EchoClient object.
Protocols::EchoClient gEchoClient;

SecureSessionMgr<Transport::UDP> gSessionManager;

Inet::IPAddress gDestAddr;

// The last time a CHIP Echo was attempted to be sent.
uint64_t gLastEchoTime = 0;

// True, if the EchoClient is waiting for an EchoResponse
// after sending an EchoRequest, false otherwise.
bool gWaitingForEchoResp = false;

// Count of the number of EchoRequests sent.
uint64_t gEchoCount = 0;

// Count of the number of EchoResponses received.
uint64_t gEchoRespCount = 0;

uint64_t Now(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    return static_cast<uint64_t>((now.tv_sec * 1000000) + now.tv_usec);
}

bool EchoIntervalExpired(void)
{
    return (Now() >= gLastEchoTime + gEchoInterval);
}

System::PacketBuffer * FormulateEchoRequestBuffer()
{
    System::PacketBuffer * payloadBuf = System::PacketBuffer::New();

    if (payloadBuf == NULL)
    {
        printf("Unable to allocate PacketBuffer\n");
    }
    else
    {
        // Add some application payload data in the buffer.
        char * p    = reinterpret_cast<char *>(payloadBuf->Start());
        int32_t len = snprintf(p, CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE, "Echo Message %" PRIu64 "\n", gEchoCount);

        // Set the datalength in the buffer appropriately.
        payloadBuf->SetDataLength((uint16_t) len);
    }

    return payloadBuf;
}

CHIP_ERROR SendEchoRequest(void)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    System::PacketBuffer * payloadBuf = NULL;

    gLastEchoTime = Now();

    payloadBuf = FormulateEchoRequestBuffer();
    if (payloadBuf == NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    printf("\nSend echo request message to Node: %lu\n", kServerDeviceId);

    err = gEchoClient.SendEchoRequest(kServerDeviceId, payloadBuf);

    // Set the local buffer to NULL after passing it down to
    // the lower layers who are now responsible for freeing
    // the buffer.
    payloadBuf = NULL;

    if (err == CHIP_NO_ERROR)
    {
        gWaitingForEchoResp = true;
        gEchoCount++;
    }
    else
    {
        printf("Send echo request failed, err: %s\n", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SecurePairingUsingTestSecret * testSecurePairingSecret = Platform::New<SecurePairingUsingTestSecret>(
        Optional<NodeId>::Value(kServerDeviceId), static_cast<uint16_t>(0), static_cast<uint16_t>(0));
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID)),
        testSecurePairingSecret);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", ErrorStr(err));
        gLastEchoTime = Now();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void HandleEchoResponseReceived(NodeId nodeId, System::PacketBuffer * payload)
{
    uint32_t respTime    = Now();
    uint32_t transitTime = respTime - gLastEchoTime;

    gWaitingForEchoResp = false;
    gEchoRespCount++;

    printf("Echo Response from node %lu : %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", nodeId, gEchoRespCount, gEchoCount,
           static_cast<double>(gEchoRespCount) * 100 / gEchoCount, payload->DataLength(), static_cast<double>(transitTime) / 1000);
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (argc <= 1)
    {
        printf("Missing Echo Server IP address\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (!Inet::IPAddress::FromString(argv[1], gDestAddr))
    {
        printf("Invalid Echo Server IP address: %s\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Initialize the CHIP stack as the client.
    InitializeChip();

    // Initialize Secure Session Manager.
    err = gSessionManager.Init(kClientDeviceId, &DeviceLayer::SystemLayer,
                               Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                                   .SetAddressType(Inet::kIPAddressType_IPv4)
                                   .SetListenPort(ECHO_CLIENT_PORT));
    SuccessOrExit(err);

    // Initialize Exchange Manager.
    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    // Initialize the EchoClient application.
    err = gEchoClient.Init(&gExchangeManager);
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    // Connection has been established. Now send the EchoRequests.
    for (unsigned int i = 0; i < kMaxEchoCount; i++)
    {
        // Send an EchoRequest message.
        if (SendEchoRequest() != CHIP_NO_ERROR)
        {
            printf("Send request failed: %s\n", ErrorStr(err));
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

    // Shutdown the EchoClient.
    gEchoClient.Shutdown();

    // Shutdown the CHIP stack.
    ShutdownChip();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("ChipEchoClient failed: %s\n", ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
