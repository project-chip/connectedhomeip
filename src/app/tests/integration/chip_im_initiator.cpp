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

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/integration/common.h>
#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>

#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PASESession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#define IM_CLIENT_PORT (CHIP_PORT + 1)

namespace {
// Max value for the number of message request sent.
constexpr size_t kMaxMessageCount = 3;

// The CHIP Message interval time in milliseconds.
constexpr int32_t gMessageInterval = 1000;

constexpr chip::Transport::AdminId gAdminId = 0;

// The CommandSender object.
chip::app::CommandSender * gpCommandSender = nullptr;

// The ReadClient object.
chip::app::ReadClient * gpReadClient = nullptr;

chip::TransportMgr<chip::Transport::UDP> gTransportManager;

chip::SecureSessionMgr gSessionManager;

chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Command was attempted to be sent.
uint64_t gLastMessageTime = 0;

// True, if the CommandSender is waiting for an CommandResponse
// after sending an CommandRequest, false otherwise.
bool gWaitingForCommandResp = false;

// True, if the ReadClient is waiting for an Report Data
// after sending an ReadRequest, false otherwise.
bool gWaitingForReadResp = false;

// Count of the number of CommandRequests sent.
uint64_t gCommandCount = 0;

// Count of the number of CommandResponses received.
uint64_t gCommandRespCount = 0;

// Count of the number of CommandRequests sent.
uint64_t gReadCount = 0;

// Count of the number of CommandResponses received.
uint64_t gReadRespCount = 0;

chip::app::EventPathParams gEventPathParams(1, 1, 2, 1);

bool MessageIntervalExpired(void)
{
    uint64_t now = chip::System::Timer::GetCurrentEpoch();

    return (now >= gLastMessageTime + gMessageInterval);
}

CHIP_ERROR SendCommandRequest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    gLastMessageTime = chip::System::Timer::GetCurrentEpoch();

    printf("\nSend invoke command request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::Command::CommandParams CommandParams = { kTestEndPointId, // Endpoint
                                                        kTestGroupId,    // GroupId
                                                        kTestClusterId,  // ClusterId
                                                        kTestCommandId,  // CommandId
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

    err = gpCommandSender->SendCommandRequest(chip::kTestDeviceNodeId, gAdminId);
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

CHIP_ERROR SendReadRequest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    gLastMessageTime = chip::System::Timer::GetCurrentEpoch();

    printf("\nSend read request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = gpReadClient->SendReadRequest(chip::kTestDeviceNodeId, gAdminId);
    SuccessOrExit(err);

    if (err == CHIP_NO_ERROR)
    {
        gWaitingForReadResp = true;
        gReadCount++;
    }
    else
    {
        printf("Send read request failed, err: %s\n", chip::ErrorStr(err));
    }
exit:
    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<chip::SecurePairingUsingTestSecret>();
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(chip::Optional<chip::Transport::PeerAddress>::Value(
                                         chip::Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID)),
                                     chip::kTestDeviceNodeId, testSecurePairingSecret,
                                     chip::SecureSessionMgr::PairingDirection::kInitiator, gAdminId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastMessageTime = chip::System::Timer::GetCurrentEpoch();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void HandleReadComplete()
{
    uint32_t respTime    = chip::System::Timer::GetCurrentEpoch();
    uint32_t transitTime = respTime - gLastMessageTime;

    gWaitingForReadResp = false;
    gReadRespCount++;

    printf("Read Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gReadRespCount, gReadCount,
           static_cast<double>(gReadRespCount) * 100 / gReadCount, static_cast<double>(transitTime) / 1000);
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate
{
public:
    void HandleIMCallBack(chip::app::InteractionModelDelegate::CallbackId aCallbackId,
                          const chip::app::InteractionModelDelegate::InParam & aInParam,
                          chip::app::InteractionModelDelegate::OutParam & aOutParam)
    {
        switch (aCallbackId)
        {
        case chip::app::InteractionModelDelegate::CallbackId::kReadRequestPrepareNeeded: {
            aOutParam.mReadRequestPrepareNeeded.eventPathParamsList     = &gEventPathParams;
            aOutParam.mReadRequestPrepareNeeded.eventPathParamsListSize = 1;

            ChipLogProgress(DataManagement, "Sending outbound read request (path count %" PRIu16 ")",
                            aOutParam.mReadRequestPrepareNeeded.eventPathParamsListSize);

            break;
        }
        case chip::app::InteractionModelDelegate::CallbackId::kEventStreamReceived: {
            ChipLogProgress(DataManagement, "Received Event stream");

            break;
        }
        case chip::app::InteractionModelDelegate::CallbackId::kReportProcessed: {
            aOutParam.mReadRequestPrepareNeeded.eventPathParamsList     = &gEventPathParams;
            aOutParam.mReadRequestPrepareNeeded.eventPathParamsListSize = 1;

            HandleReadComplete();

            break;
        }
        default:
            chip::app::InteractionModelDelegate::DefaultCallbackIdHandler(aCallbackId, aInParam, aOutParam);
            break;
        }
        return;
    }
};

} // namespace

namespace chip {
namespace app {

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj)
{
    if (aClusterId != kTestClusterId || aCommandId != kTestCommandId || aEndPointId != kTestEndPointId)
    {
        return;
    }

    uint32_t respTime    = chip::System::Timer::GetCurrentEpoch();
    uint32_t transitTime = respTime - gLastMessageTime;

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }
    gWaitingForCommandResp = false;
    gCommandRespCount++;

    printf("Command Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gCommandRespCount, gCommandCount,
           static_cast<double>(gCommandRespCount) * 100 / gCommandCount, static_cast<double>(transitTime) / 1000);
}
} // namespace app
} // namespace chip

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MockInteractionModelApp mockDelegate;
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

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

    err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager, &admins);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &mockDelegate);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP im responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&gpCommandSender);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->NewReadClient(&gpReadClient, &mockDelegate);
    SuccessOrExit(err);

    // Connection has been established. Now send the CommandRequests.
    for (unsigned int i = 0; i < kMaxMessageCount; i++)
    {
        err = SendCommandRequest();
        if (err != CHIP_NO_ERROR)
        {
            printf("Send command request failed: %s\n", chip::ErrorStr(err));
            goto exit;
        }

        // Wait for response until the Message interval.
        while (!MessageIntervalExpired())
        {
            DriveIO();
        }

        // Check if expected response was received.
        if (gWaitingForCommandResp)
        {
            printf("Invoke Command: No response received\n");
            gWaitingForCommandResp = false;
        }
    }

    // Connection has been established. Now send the ReadRequests.
    for (unsigned int i = 0; i < kMaxMessageCount; i++)
    {
        err = SendReadRequest();
        if (err != CHIP_NO_ERROR)
        {
            printf("Send read request failed: %s\n", chip::ErrorStr(err));
            goto exit;
        }

        // Wait for response until the Message interval.
        while (!MessageIntervalExpired())
        {
            DriveIO();
        }

        // Check if expected response was received.
        if (gWaitingForReadResp)
        {
            printf("read request: No response received\n");
            gWaitingForReadResp = false;
        }
    }

    gpCommandSender->Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    ShutdownChip();

exit:
    if (err != CHIP_NO_ERROR || (gCommandRespCount != kMaxMessageCount))
    {
        printf("ChipCommandSender failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    if (err != CHIP_NO_ERROR || (gReadRespCount != kMaxMessageCount))
    {
        printf("ChipReadClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }
    printf("Test success \n");
    return EXIT_SUCCESS;
}
