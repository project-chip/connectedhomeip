/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <chrono>
#include <condition_variable>
#include <core/CHIPCore.h>
#include <mutex>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#define IM_CLIENT_PORT (CHIP_PORT + 1)

namespace {
// Max value for the number of message request sent.

constexpr size_t kMaxCommandMessageCount    = 3;
constexpr size_t kMaxReadMessageCount       = 3;
constexpr int32_t gMessageIntervalSeconds   = 1;
constexpr chip::Transport::AdminId gAdminId = 0;

// The CommandSender object.
chip::app::CommandSender * gpCommandSender = nullptr;

// The ReadClient object.
chip::app::ReadClient * gpReadClient = nullptr;

chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecureSessionMgr gSessionManager;
chip::secure_channel::MessageCounterManager gMessageCounterManager;

chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Command was attempted to be sent.
uint64_t gLastMessageTime = 0;

// Count of the number of CommandRequests sent.
uint64_t gCommandCount = 0;

// Count of the number of CommandResponses received.
uint64_t gCommandRespCount = 0;

// Count of the number of CommandRequests sent.
uint64_t gReadCount = 0;

// Count of the number of CommandResponses received.
uint64_t gReadRespCount = 0;

std::condition_variable gCond;

CHIP_ERROR SendCommandRequest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    gLastMessageTime = chip::System::Timer::GetCurrentEpoch();

    printf("\nSend invoke command request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::CommandPathParams commandPathParams = { kTestEndPointId, // Endpoint
                                                       kTestGroupId,    // GroupId
                                                       kTestClusterId,  // ClusterId
                                                       kTestCommandId,  // CommandId
                                                       chip::app::CommandPathFlags::kEndpointIdValid };

    // Add command data here

    uint8_t effectIdentifier = 1; // Dying light
    uint8_t effectVariant    = 1;
    chip::TLV::TLVWriter * writer;

    err = gpCommandSender->PrepareCommand(&commandPathParams);
    SuccessOrExit(err);

    writer = gpCommandSender->GetCommandDataElementTLVWriter();
    err    = writer->Put(chip::TLV::ContextTag(1), effectIdentifier);
    SuccessOrExit(err);

    err = writer->Put(chip::TLV::ContextTag(2), effectVariant);
    SuccessOrExit(err);

    err = gpCommandSender->FinishCommand();
    SuccessOrExit(err);

    err = gpCommandSender->SendCommandRequest(chip::kTestDeviceNodeId, gAdminId);
    SuccessOrExit(err);

    if (err == CHIP_NO_ERROR)
    {
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

    err = gpReadClient->SendReadRequest(chip::kTestDeviceNodeId, gAdminId, nullptr, 0);
    SuccessOrExit(err);

    if (err == CHIP_NO_ERROR)
    {
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

    gReadRespCount++;

    printf("Read Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gReadRespCount, gReadCount,
           static_cast<double>(gReadRespCount) * 100 / gReadCount, static_cast<double>(transitTime) / 1000);

    gCond.notify_one();
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR EventStreamReceived(const chip::Messaging::ExchangeContext * apExchangeContext,
                                   chip::TLV::TLVReader * apEventListReader) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReportProcessed(const chip::app::ReadClient * apReadClient) override
    {
        HandleReadComplete();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReportError(const chip::app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        printf("ReportError with err %d", aError);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CommandResponseStatus(const chip::app::CommandSender * apCommandSender,
                                     const chip::Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                     const uint32_t aProtocolId, const uint16_t aProtocolCode, const chip::EndpointId aEndpointId,
                                     const chip::ClusterId aClusterId, const chip::CommandId aCommandId,
                                     uint8_t aCommandIndex) override
    {
        printf("CommandResponseStatus with GeneralCode %d, ProtocolId %d, ProtocolCode %d, EndpointId %d, ClusterId %d, CommandId "
               "%d, CommandIndex %d",
               static_cast<uint16_t>(aGeneralCode), aProtocolId, aProtocolCode, aEndpointId, aClusterId, aCommandId, aCommandIndex);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommandResponseProcessed(const chip::app::CommandSender * apCommandSender) override
    {

        uint32_t respTime    = chip::System::Timer::GetCurrentEpoch();
        uint32_t transitTime = respTime - gLastMessageTime;

        gCommandRespCount++;

        printf("Command Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gCommandRespCount, gCommandCount,
               static_cast<double>(gCommandRespCount) * 100 / gCommandCount, static_cast<double>(transitTime) / 1000);
        gCond.notify_one();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommandResponseProtocolError(const chip::app::CommandSender * apCommandSender, uint8_t aCommandIndex) override
    {
        printf("CommandResponseProtocolError happens with CommandIndex %d", aCommandIndex);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommandResponseError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        printf("CommandResponseError happens with %d", aError);
        return aError;
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

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }
}

} // namespace app
} // namespace chip

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
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

    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    err = gTransportManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
                                     .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                     .SetListenPort(IM_CLIENT_PORT));
    SuccessOrExit(err);

    err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager, &admins,
                               &gMessageCounterManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &mockDelegate);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP im responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&gpCommandSender);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->NewReadClient(&gpReadClient);
    SuccessOrExit(err);

    // Connection has been established. Now send the CommandRequests.
    for (unsigned int i = 0; i < kMaxCommandMessageCount; i++)
    {
        err = SendCommandRequest();
        if (err != CHIP_NO_ERROR)
        {
            printf("Send command request failed: %s\n", chip::ErrorStr(err));
            goto exit;
        }

        if (gCond.wait_for(lock, std::chrono::seconds(gMessageIntervalSeconds)) == std::cv_status::timeout)
        {
            printf("Invoke Command: No response received\n");
        }
    }

    // Connection has been established. Now send the ReadRequests.
    for (unsigned int i = 0; i < kMaxReadMessageCount; i++)
    {
        err = SendReadRequest();
        if (err != CHIP_NO_ERROR)
        {
            printf("Send read request failed: %s\n", chip::ErrorStr(err));
            goto exit;
        }

        if (gCond.wait_for(lock, std::chrono::seconds(gMessageIntervalSeconds)) == std::cv_status::timeout)
        {
            printf("read request: No response received\n");
        }
    }

    gpCommandSender->Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    ShutdownChip();

exit:
    if (err != CHIP_NO_ERROR || (gCommandRespCount != kMaxCommandMessageCount))
    {
        printf("ChipCommandSender failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    if (err != CHIP_NO_ERROR || (gReadRespCount != kMaxReadMessageCount))
    {
        printf("ChipReadClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }
    printf("Test success \n");
    return EXIT_SUCCESS;
}
