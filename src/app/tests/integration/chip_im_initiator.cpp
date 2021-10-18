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

#include "system/SystemClock.h"
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/integration/common.h>
#include <chrono>
#include <lib/core/CHIPCore.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/TypeTraits.h>
#include <memory>
#include <mutex>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>

#define IM_CLIENT_PORT (CHIP_PORT + 1)

namespace {
// Max value for the number of message request sent.

constexpr size_t kMaxCommandMessageCount          = 3;
constexpr size_t kTotalFailureCommandMessageCount = 1;
constexpr size_t kMaxReadMessageCount             = 3;
constexpr size_t kMaxWriteMessageCount            = 3;
constexpr uint32_t gMessageIntervalMsec           = 1200;
constexpr uint32_t gMessageTimeoutMsec            = 1000;
constexpr chip::FabricIndex gFabricIndex          = 0;
constexpr size_t kMaxSubMessageCount              = 1;
constexpr int32_t gMessageIntervalSeconds         = 1;
constexpr uint64_t gSubMaxReport                  = 5;

chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Command was attempted to be sent.
uint64_t gLastMessageTime = 0;

// Count of the number of CommandRequests sent.
uint64_t gCommandCount = 0;

// Count of the number of CommandResponses received.
uint64_t gCommandRespCount = 0;

// Count of the number of ReadRequests sent.
uint64_t gReadCount = 0;

// Count of the number of ReadResponses received.
uint64_t gReadRespCount = 0;

// Count of the number of WriteRequests sent.
uint64_t gWriteCount = 0;

// Count of the number of WriteResponses received.
uint64_t gWriteRespCount = 0;

// Count of the number of SubscribeRequests sent.
uint64_t gSubCount = 0;

// Count of the number of SubscribeResponses received.
uint64_t gSubRespCount = 0;

// Count of the number of reports for subscription.
uint64_t gSubReportCount = 0;

// Whether the last command successed.
enum class TestCommandResult : uint8_t
{
    kUndefined,
    kSuccess,
    kFailure
};

TestCommandResult gLastCommandResult = TestCommandResult::kUndefined;

void HandleReadComplete()
{
    auto respTime    = chip::System::SystemClock().GetMonotonicMilliseconds();
    auto transitTime = respTime - gLastMessageTime;

    gReadRespCount++;

    printf("Read Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gReadRespCount, gReadCount,
           static_cast<double>(gReadRespCount) * 100 / static_cast<double>(gReadCount), static_cast<double>(transitTime) / 1000);
}

void HandleWriteComplete()
{
    auto respTime    = chip::System::SystemClock().GetMonotonicMilliseconds();
    auto transitTime = respTime - gLastMessageTime;

    gWriteRespCount++;

    printf("Write Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gWriteRespCount, gWriteCount,
           static_cast<double>(gWriteRespCount) * 100 / static_cast<double>(gWriteCount), static_cast<double>(transitTime) / 1000);
}

void HandleSubscribeReportComplete()
{
    auto respTime    = chip::System::SystemClock().GetMonotonicMilliseconds();
    auto transitTime = respTime - gLastMessageTime;

    gSubRespCount++;

    printf("Subscribe Complete: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gSubRespCount, gSubCount,
           static_cast<double>(gSubRespCount) * 100 / static_cast<double>(gSubCount), static_cast<double>(transitTime) / 1000);
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate, public ::chip::app::CommandSender::Callback
{
public:
    CHIP_ERROR WriteResponseProcessed(const chip::app::WriteClient * apWriteClient) override
    {
        HandleWriteComplete();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EventStreamReceived(const chip::Messaging::ExchangeContext * apExchangeContext,
                                   chip::TLV::TLVReader * apEventListReader) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReportProcessed(const chip::app::ReadClient * apReadClient) override
    {
        if (apReadClient->IsSubscriptionType())
        {
            gSubReportCount++;
            if (gSubReportCount == gSubMaxReport)
            {
                HandleSubscribeReportComplete();
            }
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadError(const chip::app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        printf("ReadError with err %" CHIP_ERROR_FORMAT, aError.Format());
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReadDone(const chip::app::ReadClient * apReadClient) override
    {
        if (!apReadClient->IsSubscriptionType())
        {
            HandleReadComplete();
        }
        return CHIP_NO_ERROR;
    }

    void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                    chip::TLV::TLVReader * aData) override
    {
        printf("Command Response Success with EndpointId %d, ClusterId %d, CommandId %d", aPath.mEndpointId, aPath.mClusterId,
               aPath.mCommandId);

        gLastCommandResult = TestCommandResult::kSuccess;
        auto respTime      = chip::System::SystemClock().GetMonotonicMilliseconds();
        auto transitTime   = respTime - gLastMessageTime;

        gCommandRespCount++;

        printf("Command Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) time=%.3fms\n", gCommandRespCount, gCommandCount,
               static_cast<double>(gCommandRespCount) * 100 / static_cast<double>(gCommandCount),
               static_cast<double>(transitTime) / 1000);
    }
    void OnError(const chip::app::CommandSender * apCommandSender, chip::Protocols::InteractionModel::Status aStatus,
                 CHIP_ERROR aError) override
    {
        gCommandRespCount += (aError == CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
        gLastCommandResult = TestCommandResult::kFailure;
        printf("CommandResponseError happens with %" CHIP_ERROR_FORMAT, aError.Format());
    }
    void OnDone(chip::app::CommandSender * apCommandSender) override {}
};

MockInteractionModelApp gMockDelegate;

void CommandRequestTimerHandler(chip::System::Layer * systemLayer, void * appState);
void BadCommandRequestTimerHandler(chip::System::Layer * systemLayer, void * appState);
void ReadRequestTimerHandler(chip::System::Layer * systemLayer, void * appState);
void WriteRequestTimerHandler(chip::System::Layer * systemLayer, void * appState);
void SubscribeRequestTimerHandler(chip::System::Layer * systemLayer, void * appState);

CHIP_ERROR SendCommandRequest(std::unique_ptr<chip::app::CommandSender> && commandSender)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_INCORRECT_STATE);

    gLastMessageTime = chip::System::SystemClock().GetMonotonicMilliseconds();

    printf("\nSend invoke command request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::CommandPathParams commandPathParams = { kTestEndpointId, 0,
                                                       kTestClusterId, // ClusterId
                                                       kTestCommandId, // CommandId
                                                       chip::app::CommandPathFlags::kEndpointIdValid };

    // Add command data here

    uint8_t effectIdentifier = 1; // Dying light
    uint8_t effectVariant    = 1;
    chip::TLV::TLVWriter * writer;

    err = commandSender->PrepareCommand(commandPathParams);
    SuccessOrExit(err);

    writer = commandSender->GetCommandDataElementTLVWriter();
    err    = writer->Put(chip::TLV::ContextTag(1), effectIdentifier);
    SuccessOrExit(err);

    err = writer->Put(chip::TLV::ContextTag(2), effectVariant);
    SuccessOrExit(err);

    err = commandSender->FinishCommand();
    SuccessOrExit(err);

    err = commandSender->SendCommandRequest(chip::kTestDeviceNodeId, gFabricIndex, chip::Optional<chip::SessionHandle>::Missing(),
                                            gMessageTimeoutMsec);
    SuccessOrExit(err);

    gCommandCount++;
    commandSender.release();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Send invoke command request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}

CHIP_ERROR SendBadCommandRequest(std::unique_ptr<chip::app::CommandSender> && commandSender)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_INCORRECT_STATE);

    gLastMessageTime = chip::System::SystemClock().GetMonotonicMilliseconds();

    printf("\nSend invoke command request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::CommandPathParams commandPathParams = { 0xDE,   // Bad Endpoint
                                                       0xADBE, // Bad GroupId
                                                       0xEFCA, // Bad ClusterId
                                                       0xFE,   // Bad CommandId
                                                       chip::app::CommandPathFlags::kEndpointIdValid };

    err = commandSender->PrepareCommand(commandPathParams);
    SuccessOrExit(err);

    err = commandSender->FinishCommand();
    SuccessOrExit(err);

    err = commandSender->SendCommandRequest(chip::kTestDeviceNodeId, gFabricIndex, chip::Optional<chip::SessionHandle>::Missing(),
                                            gMessageTimeoutMsec);
    SuccessOrExit(err);
    gCommandCount++;
    commandSender.release();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Send invoke command request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}

CHIP_ERROR SendReadRequest()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = kTestNodeId;
    eventPathParams[0].mEndpointId = kTestEndpointId;
    eventPathParams[0].mClusterId  = kTestClusterId;
    eventPathParams[0].mEventId    = kTestChangeEvent1;

    eventPathParams[1].mNodeId     = kTestNodeId;
    eventPathParams[1].mEndpointId = kTestEndpointId;
    eventPathParams[1].mClusterId  = kTestClusterId;
    eventPathParams[1].mEventId    = kTestChangeEvent2;

    chip::app::AttributePathParams attributePathParams(chip::kTestDeviceNodeId, kTestEndpointId, kTestClusterId, 1, 0,
                                                       chip::app::AttributePathParams::Flags::kFieldIdValid);

    printf("\nSend read request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    chip::app::ReadPrepareParams readPrepareParams(chip::SessionHandle(chip::kTestDeviceNodeId, 0, 0, gFabricIndex));
    readPrepareParams.mTimeout                     = gMessageTimeoutMsec;
    readPrepareParams.mpAttributePathParamsList    = &attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;
    readPrepareParams.mpEventPathParamsList        = eventPathParams;
    readPrepareParams.mEventPathParamsListSize     = 2;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams);
    SuccessOrExit(err);

exit:
    if (err == CHIP_NO_ERROR)
    {
        gReadCount++;
    }
    else
    {
        printf("Send read request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}

CHIP_ERROR SendWriteRequest(chip::app::WriteClientHandle & apWriteClient)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter * writer;
    gLastMessageTime = chip::System::SystemClock().GetMonotonicMilliseconds();
    chip::app::AttributePathParams attributePathParams;

    printf("\nSend write request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mListIndex  = 5;
    attributePathParams.mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    SuccessOrExit(err = apWriteClient->PrepareAttribute(attributePathParams));

    writer = apWriteClient->GetAttributeDataElementTLVWriter();

    SuccessOrExit(err = writer->PutBoolean(chip::TLV::ContextTag(chip::app::AttributeDataElement::kCsTag_Data), true));
    SuccessOrExit(err = apWriteClient->FinishAttribute());
    SuccessOrExit(err = apWriteClient.SendWriteRequest(chip::kTestDeviceNodeId, gFabricIndex,
                                                       chip::Optional<chip::SessionHandle>::Missing(), gMessageTimeoutMsec));

    gWriteCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Send read request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}

CHIP_ERROR SendSubscribeRequest()
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    gLastMessageTime = chip::System::SystemClock().GetMonotonicMilliseconds();

    chip::app::ReadPrepareParams readPrepareParams(chip::SessionHandle(chip::kTestDeviceNodeId, 0, 0, gFabricIndex));
    chip::app::EventPathParams eventPathParams[2];
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mNodeId     = kTestNodeId;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestChangeEvent1;

    readPrepareParams.mpEventPathParamsList[1].mNodeId     = kTestNodeId;
    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestChangeEvent2;

    readPrepareParams.mEventPathParamsListSize = 2;

    readPrepareParams.mpAttributePathParamsList                = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mNodeId     = chip::kTestDeviceNodeId;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mFieldId    = 1;
    readPrepareParams.mpAttributePathParamsList[0].mListIndex  = 0;
    readPrepareParams.mpAttributePathParamsList[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = chip::app::InteractionModelEngine::GetInstance()->SendSubscribeRequest(readPrepareParams);
    SuccessOrExit(err);

    gSubCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Send subscribe request failed, err: %s\n", chip::ErrorStr(err));
    }
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
                                     chip::kTestDeviceNodeId, testSecurePairingSecret, chip::CryptoContext::SessionRole::kInitiator,
                                     gFabricIndex);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastMessageTime = chip::System::SystemClock().GetMonotonicMilliseconds();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void CommandRequestTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (gCommandRespCount != gCommandCount)
    {
        printf("No response received\n");

        // Set gCommandRespCount to gCommandCount to start next iteration if there is any.
        gCommandRespCount = gCommandCount;
    }

    if (gCommandRespCount < kMaxCommandMessageCount)
    {
        auto commandSender = std::make_unique<chip::app::CommandSender>(&gMockDelegate, &gExchangeManager);
        VerifyOrExit(commandSender != nullptr, err = CHIP_ERROR_NO_MEMORY);

        err = SendCommandRequest(std::move(commandSender));
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to send command request with error: %s\n", chip::ErrorStr(err)));

        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, CommandRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }
    else
    {
        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, BadCommandRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

void BadCommandRequestTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    // Test with invalid endpoint / cluster / command combination.
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto commandSender = std::make_unique<chip::app::CommandSender>(&gMockDelegate, &gExchangeManager);
    VerifyOrExit(commandSender != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = SendBadCommandRequest(std::move(commandSender));
    VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to send bad command request with error: %s\n", chip::ErrorStr(err)));

    err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, ReadRequestTimerHandler, NULL);
    VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

void ReadRequestTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (gReadRespCount != gReadCount)
    {
        printf("No response received\n");

        // Set gReadRespCount to gReadCount to start next iteration if there is any.
        gReadRespCount = gReadCount;
    }

    if (gReadRespCount < kMaxReadMessageCount)
    {
        err = SendReadRequest();
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to send read request with error: %s\n", chip::ErrorStr(err)));

        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, ReadRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }
    else
    {
        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, WriteRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

void WriteRequestTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (gWriteRespCount != gWriteCount)
    {
        printf("No response received\n");

        // Set gWriteRespCount to gWriteCount to start next iteration if there is any.
        gWriteRespCount = gWriteCount;
    }

    if (gWriteRespCount < kMaxWriteMessageCount)
    {
        chip::app::WriteClientHandle writeClient;
        err = chip::app::InteractionModelEngine::GetInstance()->NewWriteClient(writeClient);
        SuccessOrExit(err);

        err = SendWriteRequest(writeClient);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to send write request with error: %s\n", chip::ErrorStr(err)));

        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalMsec, WriteRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }
    else
    {
        err = chip::DeviceLayer::SystemLayer().StartTimer(gMessageIntervalSeconds * 1000, SubscribeRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

void SubscribeRequestTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (gSubRespCount != gSubCount)
    {
        printf("No response received\n");

        // Set gSubRespCount to gSubCount to start next iteration if there is any.
        gSubRespCount = gSubCount;
    }

    if (gSubRespCount < kMaxSubMessageCount)
    {
        err = SendSubscribeRequest();
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to send write request with error: %s\n", chip::ErrorStr(err)));

        err = chip::DeviceLayer::SystemLayer().StartTimer(20 * 1000, SubscribeRequestTimerHandler, NULL);
        VerifyOrExit(err == CHIP_NO_ERROR, printf("Failed to schedule timer with error: %s\n", chip::ErrorStr(err)));
    }
    else
    {
        // Complete all tests.
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}
} // namespace

namespace chip {
namespace app {

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Always return true in test.
    return true;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    // Nothing todo.
}

void DispatchSingleClusterResponseCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                          CommandSender * apCommandObj)
{
    if (aCommandPath.mClusterId != kTestClusterId || aCommandPath.mCommandId != kTestCommandId ||
        aCommandPath.mEndpointId != kTestEndpointId)
    {
        return;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    gLastCommandResult = TestCommandResult::kSuccess;
}

CHIP_ERROR ReadSingleClusterData(const ConcreteAttributePath & aPath, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    // We do not really care about the value, just return a not found status code.
    VerifyOrReturnError(apWriter != nullptr, CHIP_NO_ERROR);
    return apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
                         Protocols::InteractionModel::Status::UnsupportedAttribute);
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler *)
{
    if (aClusterInfo.mClusterId != kTestClusterId || aClusterInfo.mEndpointId != kTestEndpointId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

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

    static_assert(gMessageIntervalMsec > gMessageTimeoutMsec, "Interval period too small");

    InitializeChip();

    err = gTransportManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
                                     .SetAddressType(chip::Inet::kIPAddressType_IPv6)
                                     .SetListenPort(IM_CLIENT_PORT));
    SuccessOrExit(err);

    err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gTransportManager, &gMessageCounterManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &gMockDelegate);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP im responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    err = chip::DeviceLayer::SystemLayer().StartTimer(0, CommandRequestTimerHandler, NULL);
    SuccessOrExit(err);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    ShutdownChip();
exit:
    if (err != CHIP_NO_ERROR || (gCommandRespCount != kMaxCommandMessageCount + kTotalFailureCommandMessageCount))
    {
        printf("ChipCommandSender failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    if (err != CHIP_NO_ERROR || (gReadRespCount != kMaxReadMessageCount))
    {
        printf("ChipReadClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    if (err != CHIP_NO_ERROR || (gWriteRespCount != kMaxWriteMessageCount))
    {
        printf("ChipWriteClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    printf("Test success \n");
    return EXIT_SUCCESS;
}
