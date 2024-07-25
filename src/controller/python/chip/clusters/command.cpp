/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <memory>
#include <type_traits>
#include <unordered_map>

#include <app/CommandSender.h>
#include <app/DeviceProxy.h>
#include <lib/support/CodeUtils.h>

#include <controller/python/chip/interaction_model/Delegate.h>
#include <controller/python/chip/native/PyChipError.h>
#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

using PyObject = void *;

extern "C" {
PyChipError pychip_CommandSender_SendCommand(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                             chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                                             const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs,
                                             uint16_t busyWaitMs, bool suppressResponse);

PyChipError pychip_CommandSender_SendBatchCommands(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                                   uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse,
                                                   chip::python::PyInvokeRequestData * batchCommandData, size_t length);

PyChipError pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke(
    void * appContext, DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
    const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse);

PyChipError pychip_CommandSender_SendGroupCommand(chip::GroupId groupId, chip::Controller::DeviceCommissioner * devCtrl,
                                                  chip::ClusterId clusterId, chip::CommandId commandId, const uint8_t * payload,
                                                  size_t length, uint16_t busyWaitMs);
}

namespace chip {
namespace python {

using OnCommandSenderResponseCallback     = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                 chip::CommandId commandId, size_t index,
                                                 std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                                 chip::ClusterStatus clusterStatus, const uint8_t * payload, uint32_t length);
using OnCommandSenderErrorCallback        = void (*)(PyObject appContext,
                                              std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                              chip::ClusterStatus clusterStatus, PyChipError chiperror);
using OnCommandSenderDoneCallback         = void (*)(PyObject appContext);
using TestOnlyOnCommandSenderDoneCallback = void (*)(PyObject appContext, python::TestOnlyPyOnDoneInfo testOnlyDoneInfo);

OnCommandSenderResponseCallback gOnCommandSenderResponseCallback         = nullptr;
OnCommandSenderErrorCallback gOnCommandSenderErrorCallback               = nullptr;
OnCommandSenderDoneCallback gOnCommandSenderDoneCallback                 = nullptr;
TestOnlyOnCommandSenderDoneCallback gTestOnlyOnCommandSenderDoneCallback = nullptr;

class CommandSenderCallback : public CommandSender::ExtendableCallback
{
public:
    CommandSenderCallback(PyObject appContext, bool isBatchedCommands, bool callTestOnlyOnDone) :
        mAppContext(appContext), mIsBatchedCommands(isBatchedCommands), mCallTestOnlyOnDone(callTestOnlyOnDone)
    {}

    void OnResponse(CommandSender * apCommandSender, const CommandSender::ResponseData & aResponseData) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        uint8_t buffer[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
        uint32_t size = 0;
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (aResponseData.data != nullptr)
        {
            // Python need to read from full TLV data the TLVReader may contain some unclean states.
            TLV::TLVWriter writer;
            writer.Init(buffer);
            err = writer.CopyContainer(TLV::AnonymousTag(), *aResponseData.data);
            if (err != CHIP_NO_ERROR)
            {
                CommandSender::ErrorData errorData = { err };
                this->OnError(apCommandSender, errorData);
                return;
            }
            size = writer.GetLengthWritten();
        }

        const app::StatusIB & statusIB = aResponseData.statusIB;

        // For legacy specific reasons when we are not processing a batch command we simply forward this to the OnError callback
        // for more information on why see https://github.com/project-chip/connectedhomeip/issues/30991.
        if (!mIsBatchedCommands && !statusIB.IsSuccess())
        {
            CommandSender::ErrorData errorData = { statusIB.ToChipError() };
            this->OnError(apCommandSender, errorData);
            return;
        }

        if (err != CHIP_NO_ERROR)
        {
            CommandSender::ErrorData errorData = { err };
            this->OnError(apCommandSender, errorData);
            return;
        }

        chip::CommandRef commandRef = aResponseData.commandRef.ValueOr(0);
        size_t index                = 0;
        err                         = GetIndexFrocommandRef(commandRef, index);
        if (err != CHIP_NO_ERROR && mIsBatchedCommands)
        {
            CommandSender::ErrorData errorData = { err };
            this->OnError(apCommandSender, errorData);
            return;
        }

        const ConcreteCommandPath & path = aResponseData.path;

        gOnCommandSenderResponseCallback(
            mAppContext, path.mEndpointId, path.mClusterId, path.mCommandId, index, to_underlying(statusIB.mStatus),
            statusIB.mClusterStatus.HasValue() ? statusIB.mClusterStatus.Value() : chip::python::kUndefinedClusterStatus, buffer,
            size);
    }

    void OnError(const CommandSender * apCommandSender, const CommandSender::ErrorData & aErrorData) override
    {
        CHIP_ERROR protocolError = aErrorData.error;
        StatusIB status(protocolError);
        gOnCommandSenderErrorCallback(mAppContext, to_underlying(status.mStatus),
                                      status.mClusterStatus.ValueOr(chip::python::kUndefinedClusterStatus),
                                      // If we have an actual IM status, pass 0
                                      // for the error code, because otherwise
                                      // the callee will think we have a stack
                                      // exception.
                                      protocolError.IsIMStatus() ? ToPyChipError(CHIP_NO_ERROR) : ToPyChipError(protocolError));
    }

    void OnDone(CommandSender * apCommandSender) override
    {
        if (mCallTestOnlyOnDone)
        {
            python::TestOnlyPyOnDoneInfo testOnlyOnDoneInfo;
            testOnlyOnDoneInfo.responseMessageCount = apCommandSender->GetInvokeResponseMessageCount();
            gTestOnlyOnCommandSenderDoneCallback(mAppContext, testOnlyOnDoneInfo);
        }
        else
        {
            gOnCommandSenderDoneCallback(mAppContext);
        }

        delete apCommandSender;
        delete this;
    };

    CHIP_ERROR GetIndexFrocommandRef(uint16_t aCommandRef, size_t & aIndex)
    {
        auto search = commandRefToIndex.find(aCommandRef);
        if (search == commandRefToIndex.end())
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        aIndex = commandRefToIndex[aCommandRef];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddCommandRefToIndexLookup(uint16_t aCommandRef, size_t aIndex)
    {
        auto search = commandRefToIndex.find(aCommandRef);
        if (search != commandRefToIndex.end())
        {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        commandRefToIndex[aCommandRef] = aIndex;
        return CHIP_NO_ERROR;
    }

private:
    PyObject mAppContext = nullptr;
    std::unordered_map<uint16_t, size_t> commandRefToIndex;
    bool mIsBatchedCommands;
    bool mCallTestOnlyOnDone;
};

PyChipError SendBatchCommandsInternal(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                      uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse,
                                      python::TestOnlyPyBatchCommandsOverrides * testOnlyOverrides,
                                      python::PyInvokeRequestData * batchCommandData, size_t length)
{
    CommandSender::ConfigParameters config;
    CHIP_ERROR err = CHIP_NO_ERROR;

    bool testOnlySuppressTimedRequestMessage = false;
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    uint16_t * testOnlyCommandRefsOverride = nullptr;
#endif

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    // Test only override validation checks and setup
    if (testOnlyOverrides != nullptr)
    {
        if (testOnlyOverrides->suppressTimedRequestMessage)
        {
            VerifyOrReturnError(timedRequestTimeoutMs == 0, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
            testOnlySuppressTimedRequestMessage = true;
        }
        if (testOnlyOverrides->overrideCommandRefsList != nullptr)
        {
            VerifyOrReturnError(length == testOnlyOverrides->overrideCommandRefsListLength,
                                ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
            testOnlyCommandRefsOverride = testOnlyOverrides->overrideCommandRefsList;
        }
    }

    if (testOnlyOverrides != nullptr && testOnlyOverrides->overrideRemoteMaxPathsPerInvoke)
    {
        config.SetRemoteMaxPathsPerInvoke(testOnlyOverrides->overrideRemoteMaxPathsPerInvoke);
    }
    else
#endif
    {
        auto remoteSessionParameters = device->GetSecureSession().Value()->GetRemoteSessionParameters();
        config.SetRemoteMaxPathsPerInvoke(remoteSessionParameters.GetMaxPathsPerInvoke());
    }

    bool isBatchedCommands  = true;
    bool callTestOnlyOnDone = testOnlyOverrides != nullptr;
    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, isBatchedCommands, callTestOnlyOnDone);

    bool isTimedRequest = timedRequestTimeoutMs != 0 || testOnlySuppressTimedRequestMessage;
    std::unique_ptr<CommandSender> sender =
        std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager(), isTimedRequest, suppressResponse);

    SuccessOrExit(err = sender->SetCommandSenderConfig(config));

    for (size_t i = 0; i < length; i++)
    {
        chip::EndpointId endpointId = batchCommandData[i].commandPath.endpointId;
        chip::ClusterId clusterId   = batchCommandData[i].commandPath.clusterId;
        chip::CommandId commandId   = batchCommandData[i].commandPath.commandId;
        void * tlv                  = batchCommandData[i].tlvData;
        size_t tlvLength            = batchCommandData[i].tlvLength;

        const uint8_t * tlvBuffer = reinterpret_cast<const uint8_t *>(tlv);

        app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, clusterId, commandId,
                                             (app::CommandPathFlags::kEndpointIdValid) };

        CommandSender::PrepareCommandParameters prepareCommandParams;
        prepareCommandParams.commandRef.SetValue(static_cast<uint16_t>(i));

        SuccessOrExit(err = sender->PrepareCommand(cmdParams, prepareCommandParams));
        {
            auto writer = sender->GetCommandDataIBTLVWriter();
            VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            TLV::TLVReader reader;
            reader.Init(tlvBuffer, static_cast<uint32_t>(tlvLength));
            reader.Next();
            SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(CommandDataIB::Tag::kFields), reader));
        }

        Optional<uint16_t> timedRequestTimeout =
            timedRequestTimeoutMs != 0 ? Optional<uint16_t>(timedRequestTimeoutMs) : Optional<uint16_t>::Missing();
        CommandSender::FinishCommandParameters finishCommandParams(timedRequestTimeout);
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (testOnlyCommandRefsOverride != nullptr)
        {
            finishCommandParams.commandRef.SetValue(testOnlyCommandRefsOverride[i]);
        }
        else
        {
            finishCommandParams.commandRef = prepareCommandParams.commandRef;
        }
        SuccessOrExit(err = sender->TestOnlyFinishCommand(finishCommandParams));

        if (testOnlyCommandRefsOverride != nullptr)
        {
            // Making sure the value we used to override CommandRef was actually used.
            VerifyOrDie(finishCommandParams.commandRef.Value() == testOnlyCommandRefsOverride[i]);
            // Ignoring the result of adding to index as the test might be trying to set duplicate CommandRefs.
            callback->AddCommandRefToIndexLookup(finishCommandParams.commandRef.Value(), i);
        }
        else
#endif
        {
            SuccessOrExit(err = callback->AddCommandRefToIndexLookup(finishCommandParams.commandRef.Value(), i));
        }
    }

    {
        Optional<System::Clock::Timeout> interactionTimeout = interactionTimeoutMs != 0
            ? MakeOptional(System::Clock::Milliseconds32(interactionTimeoutMs))
            : Optional<System::Clock::Timeout>::Missing();
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (testOnlySuppressTimedRequestMessage)
        {
            SuccessOrExit(err = sender->TestOnlyCommandSenderTimedRequestFlagWithNoTimedInvoke(device->GetSecureSession().Value(),
                                                                                               interactionTimeout));
        }
        else
#endif
        {
            SuccessOrExit(err = sender->SendCommandRequest(device->GetSecureSession().Value(), interactionTimeout));
        }
    }

    sender.release();
    callback.release();

    // TODO(#30985): Reconsider the purpose of busyWait and if it can be broken out into it's
    // own method/primitive.
    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
}

} // namespace python
} // namespace chip

using namespace chip::python;

extern "C" {
void pychip_CommandSender_InitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                        OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                        OnCommandSenderDoneCallback onCommandSenderDoneCallback,
                                        TestOnlyOnCommandSenderDoneCallback testOnlyOnCommandSenderDoneCallback)
{
    gOnCommandSenderResponseCallback     = onCommandSenderResponseCallback;
    gOnCommandSenderErrorCallback        = onCommandSenderErrorCallback;
    gOnCommandSenderDoneCallback         = onCommandSenderDoneCallback;
    gTestOnlyOnCommandSenderDoneCallback = testOnlyOnCommandSenderDoneCallback;
}

PyChipError pychip_CommandSender_SendCommand(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                             chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                                             const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs,
                                             uint16_t busyWaitMs, bool suppressResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));

    bool isBatchedCommands  = false;
    bool callTestOnlyOnDone = false;
    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, isBatchedCommands, callTestOnlyOnDone);
    std::unique_ptr<CommandSender> sender =
        std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager(),
                                        /* is timed request */ timedRequestTimeoutMs != 0, suppressResponse);

    app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, clusterId, commandId,
                                         (app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = sender->PrepareCommand(cmdParams, false));

    {
        auto writer = sender->GetCommandDataIBTLVWriter();
        TLV::TLVReader reader;
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(payload, length);
        reader.Next();
        SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(CommandDataIB::Tag::kFields), reader));
    }

    SuccessOrExit(err = sender->FinishCommand(timedRequestTimeoutMs != 0 ? Optional<uint16_t>(timedRequestTimeoutMs)
                                                                         : Optional<uint16_t>::Missing()));

    SuccessOrExit(err = sender->SendCommandRequest(device->GetSecureSession().Value(),
                                                   interactionTimeoutMs != 0
                                                       ? MakeOptional(System::Clock::Milliseconds32(interactionTimeoutMs))
                                                       : Optional<System::Clock::Timeout>::Missing()));

    sender.release();
    callback.release();

    // TODO(#30985): Reconsider the purpose of busyWait and if it can be broken out into it's
    // own method/primitive.
    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
}

PyChipError pychip_CommandSender_SendBatchCommands(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                                   uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse,
                                                   python::PyInvokeRequestData * batchCommandData, size_t length)
{
    python::TestOnlyPyBatchCommandsOverrides * testOnlyOverrides = nullptr;
    return SendBatchCommandsInternal(appContext, device, timedRequestTimeoutMs, interactionTimeoutMs, busyWaitMs, suppressResponse,
                                     testOnlyOverrides, batchCommandData, length);
}

PyChipError pychip_CommandSender_TestOnlySendBatchCommands(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                                           uint16_t interactionTimeoutMs, uint16_t busyWaitMs,
                                                           bool suppressResponse,
                                                           python::TestOnlyPyBatchCommandsOverrides testOnlyOverrides,
                                                           python::PyInvokeRequestData * batchCommandData, size_t length)
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    return SendBatchCommandsInternal(appContext, device, timedRequestTimeoutMs, interactionTimeoutMs, busyWaitMs, suppressResponse,
                                     &testOnlyOverrides, batchCommandData, length);
#else
    return ToPyChipError(CHIP_ERROR_NOT_IMPLEMENTED);
#endif
}

PyChipError pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke(
    void * appContext, DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
    const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse)
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));

    bool isBatchedCommands  = false;
    bool callTestOnlyOnDone = false;
    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, isBatchedCommands, callTestOnlyOnDone);
    std::unique_ptr<CommandSender> sender = std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager(),
                                                                            /* is timed request */ true, suppressResponse);

    app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, clusterId, commandId,
                                         (app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = sender->PrepareCommand(cmdParams, false));

    {
        auto writer = sender->GetCommandDataIBTLVWriter();
        TLV::TLVReader reader;
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(payload, length);
        reader.Next();
        SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(CommandDataIB::Tag::kFields), reader));
    }

    SuccessOrExit(err = sender->FinishCommand(false));

    SuccessOrExit(err = sender->TestOnlyCommandSenderTimedRequestFlagWithNoTimedInvoke(
                      device->GetSecureSession().Value(),
                      interactionTimeoutMs != 0 ? MakeOptional(System::Clock::Milliseconds32(interactionTimeoutMs))
                                                : Optional<System::Clock::Timeout>::Missing()));

    sender.release();
    callback.release();

    // TODO(#30985): Reconsider the purpose of busyWait and if it can be broken out into it's
    // own method/primitive.
    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
#else
    return ToPyChipError(CHIP_ERROR_NOT_IMPLEMENTED);
#endif
}

PyChipError pychip_CommandSender_SendGroupCommand(chip::GroupId groupId, chip::Controller::DeviceCommissioner * devCtrl,
                                                  chip::ClusterId clusterId, chip::CommandId commandId, const uint8_t * payload,
                                                  size_t length, uint16_t busyWaitMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();
    VerifyOrReturnError(exchangeManager != nullptr, ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    std::unique_ptr<CommandSender> sender = std::make_unique<CommandSender>(nullptr /* callback */, exchangeManager);

    app::CommandPathParams cmdParams = { groupId, clusterId, commandId, (app::CommandPathFlags::kGroupIdValid) };

    SuccessOrExit(err = sender->PrepareCommand(cmdParams, false));

    {
        auto writer = sender->GetCommandDataIBTLVWriter();
        TLV::TLVReader reader;
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(payload, length);
        reader.Next();
        SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(CommandDataIB::Tag::kFields), reader));
    }

    SuccessOrExit(err = sender->FinishCommand(Optional<uint16_t>::Missing()));

    {
        auto fabricIndex = devCtrl->GetFabricIndex();

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        SuccessOrExit(err = sender->SendGroupCommandRequest(chip::SessionHandle(session)));
    }

    // TODO(#30985): Reconsider the purpose of busyWait and if it can be broken out into it's
    // own method/primitive.
    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
}
}
