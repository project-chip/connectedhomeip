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
 *      This file implements a chip-im-initiator, for the
 *      CHIP Interaction Data Model Protocol.
 *
 *      Currently it provides simple command sender with sample cluster and command
 *
 */

#include "common.h"

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>

#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#define IM_CLIENT_PORT (CHIP_PORT + 1)

namespace {

// Max value for the number of command request sent.
constexpr size_t kMaxCommandCount = 3;

// The CHIP Command interval time in milliseconds.
constexpr int32_t gCommandInterval = 1000;

// The CommandSender object.
chip::app::CommandSender * gpCommandSender = nullptr;

chip::TransportMgr<chip::Transport::UDP> gTransportManager;

chip::SecureSessionMgr gSessionManager;

chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Command was attempted to be sent.
uint64_t gLastCommandTime = 0;

// True, if the CommandSender is waiting for an CommandResponse
// after sending an CommandRequest, false otherwise.
bool gWaitingForCommandResp = false;

// Count of the number of CommandRequests sent.
uint64_t gCommandCount = 0;

// Count of the number of CommandResponses received.
uint64_t gCommandRespCount = 0;

bool CommandIntervalExpired(void)
{
    uint64_t now = chip::System::Timer::GetCurrentEpoch();

    return (now >= gLastCommandTime + gCommandInterval);
}

CHIP_ERROR SendCommandRequest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    gLastCommandTime = chip::System::Timer::GetCurrentEpoch();

    printf("\nSend invoke command request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::Command::CommandParams CommandParams = { 1,  // Endpoint
                                                        0,  // GroupId
                                                        6,  // ClusterId
                                                        40, // CommandId
                                                        (chip::app::Command::kCommandPathFlag_EndpointIdValid) };

    // Add command data here

    uint8_t effectIdentifier = 1; // Dying light
    uint8_t effectVariant    = 1;

    chip::TLV::TLVType dummyType = chip::TLV::kTLVType_NotSpecified;

    chip::TLV::TLVWriter writer = gpCommandSender->CreateCommandDataElementTLVWriter();

    err = writer.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, dummyType);
    SuccessOrExit(err);

    err = writer.Put(chip::TLV::ContextTag(1), effectIdentifier);
    SuccessOrExit(err);

    err = writer.Put(chip::TLV::ContextTag(2), effectVariant);
    SuccessOrExit(err);

    err = writer.EndContainer(dummyType);
    SuccessOrExit(err);

    err = writer.Finalize();
    SuccessOrExit(err);

    err = gpCommandSender->AddCommand(CommandParams);
    SuccessOrExit(err);

    err = gpCommandSender->SendCommandRequest(chip::kTestDeviceNodeId);
    SuccessOrExit(err);

    if (err == CHIP_NO_ERROR)
    {
        gWaitingForCommandResp = true;
        gCommandCount++;
    }
    else
    {
        printf("Send invoke command request failed, err: %s\n", chip::ErrorStr(err));
    }
exit:
    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<chip::SecurePairingUsingTestSecret>(
        chip::Optional<chip::NodeId>::Value(chip::kTestDeviceNodeId), static_cast<uint16_t>(0), static_cast<uint16_t>(0));
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(chip::Optional<chip::Transport::PeerAddress>::Value(
                                         chip::Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID)),
                                     chip::kTestDeviceNodeId, testSecurePairingSecret);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastCommandTime = chip::System::Timer::GetCurrentEpoch();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void HandleCommandResponseReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    uint32_t respTime    = chip::System::Timer::GetCurrentEpoch();
    uint32_t transitTime = respTime - gLastCommandTime;

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }
    gWaitingForCommandResp = false;
    gCommandRespCount++;

    printf("Command Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gCommandRespCount, gCommandCount,
           static_cast<double>(gCommandRespCount) * 100 / gCommandCount, static_cast<double>(transitTime) / 1000);
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (argc <= 1)
    {
        printf("Missing Command Server IP address\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (!chip::Inet::IPAddress::FromString(argv[1], gDestAddr))
    {
        printf("Invalid Command Server IP address: %s\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    InitializeChip();

    err = gTransportManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
                                     .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                     .SetListenPort(IM_CLIENT_PORT));
    SuccessOrExit(err);

    err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(chip::kTestControllerNodeId, &gTransportManager, &gSessionManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager);
    SuccessOrExit(err);

    chip::app::InteractionModelEngine::GetInstance()->RegisterClusterCommandHandler(
        6, 40, chip::app::Command::CommandRoleId::kCommandSenderId, HandleCommandResponseReceived);

    // Start the CHIP connection to the CHIP im responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&gpCommandSender);
    SuccessOrExit(err);

    // Connection has been established. Now send the CommandRequests.
    for (unsigned int i = 0; i < kMaxCommandCount; i++)
    {
        if (SendCommandRequest() != CHIP_NO_ERROR)
        {
            printf("Send request failed: %s\n", chip::ErrorStr(err));
            break;
        }

        // Wait for response until the Command interval.
        while (!CommandIntervalExpired())
        {
            DriveIO();
        }

        // Check if expected response was received.
        if (gWaitingForCommandResp)
        {
            printf("No response received\n");
            gWaitingForCommandResp = false;
        }
    }

    chip::app::InteractionModelEngine::GetInstance()->DeregisterClusterCommandHandler(
        6, 40, chip::app::Command::CommandRoleId::kCommandSenderId);

    gpCommandSender->Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    ShutdownChip();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("ChipCommandSender failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
