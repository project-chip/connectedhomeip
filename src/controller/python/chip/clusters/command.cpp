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
                                                   size_t n, ...);

PyChipError pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke(
    void * appContext, DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
    const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse);

PyChipError pychip_CommandSender_SendGroupCommand(chip::GroupId groupId, chip::Controller::DeviceCommissioner * devCtrl,
                                                  chip::ClusterId clusterId, chip::CommandId commandId, const uint8_t * payload,
                                                  size_t length, uint16_t busyWaitMs);
}

namespace chip {
namespace python {

using OnCommandSenderResponseCallback = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                 chip::CommandId commandId, size_t index,
                                                 std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                                 chip::ClusterStatus clusterStatus, const uint8_t * payload, uint32_t length);
using OnCommandSenderErrorCallback    = void (*)(PyObject appContext,
                                              std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                              chip::ClusterStatus clusterStatus, PyChipError chiperror);
using OnCommandSenderDoneCallback     = void (*)(PyObject appContext);

OnCommandSenderResponseCallback gOnCommandSenderResponseCallback = nullptr;
OnCommandSenderErrorCallback gOnCommandSenderErrorCallback       = nullptr;
OnCommandSenderDoneCallback gOnCommandSenderDoneCallback         = nullptr;

struct __attribute__((packed)) CommandPath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
};

class CommandSenderCallback : public CommandSender::Callback
{
public:
    CommandSenderCallback(PyObject appContext, bool isBatchedCommands) :
        mAppContext(appContext), mIsBatchedCommands(isBatchedCommands)
    {}

    void OnResponseWithAdditionalData(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                      const app::StatusIB & aStatus, TLV::TLVReader * aData,
                                      const CommandSender::AdditionalResponseData & aAdditionalResponseData) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        uint8_t buffer[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
        uint32_t size = 0;
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (aData != nullptr)
        {
            // Python need to read from full TLV data the TLVReader may contain some unclean states.
            TLV::TLVWriter writer;
            writer.Init(buffer);
            err = writer.CopyContainer(TLV::AnonymousTag(), *aData);
            if (err != CHIP_NO_ERROR)
            {
                this->OnError(apCommandSender, err);
                return;
            }
            size = writer.GetLengthWritten();
        }

        if (err != CHIP_NO_ERROR)
        {
            this->OnError(apCommandSender, err);
            return;
        }

        chip::CommandRef commandRef = aAdditionalResponseData.mCommandRef.ValueOr(0);
        size_t index                = 0;
        err                         = GetIndexFromCommandRef(commandRef, index);
        if (err != CHIP_NO_ERROR && mIsBatchedCommands)
        {
            this->OnError(apCommandSender, err);
            return;
        }

        gOnCommandSenderResponseCallback(
            mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mCommandId, index, to_underlying(aStatus.mStatus),
            aStatus.mClusterStatus.HasValue() ? aStatus.mClusterStatus.Value() : chip::python::kUndefinedClusterStatus, buffer,
            size);
    }

    void OnError(const CommandSender * apCommandSender, CHIP_ERROR aProtocolError) override
    {
        StatusIB status(aProtocolError);
        gOnCommandSenderErrorCallback(mAppContext, to_underlying(status.mStatus),
                                      status.mClusterStatus.ValueOr(chip::python::kUndefinedClusterStatus),
                                      // If we have an actual IM status, pass 0
                                      // for the error code, because otherwise
                                      // the callee will think we have a stack
                                      // exception.
                                      aProtocolError.IsIMStatus() ? ToPyChipError(CHIP_NO_ERROR) : ToPyChipError(aProtocolError));
    }

    void OnDone(CommandSender * apCommandSender) override
    {
        gOnCommandSenderDoneCallback(mAppContext);
        delete apCommandSender;
        delete this;
    };

    CHIP_ERROR GetIndexFromCommandRef(uint16_t aCommandRef, size_t & aIndex)
    {
        auto search = mCommandRefToIndex.find(aCommandRef);
        if (search == mCommandRefToIndex.end())
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        aIndex = mCommandRefToIndex[aCommandRef];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddCommandRefToIndexLookup(uint16_t aCommandRef, size_t aIndex)
    {
        auto search = mCommandRefToIndex.find(aCommandRef);
        if (search != mCommandRefToIndex.end())
        {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        mCommandRefToIndex[aCommandRef] = aIndex;
        return CHIP_NO_ERROR;
    }

private:
    PyObject mAppContext = nullptr;
    std::unordered_map<uint16_t, size_t> mCommandRefToIndex;
    bool mIsBatchedCommands;
};

} // namespace python
} // namespace chip

using namespace chip::python;

extern "C" {
void pychip_CommandSender_InitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                        OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                        OnCommandSenderDoneCallback onCommandSenderDoneCallback)
{
    gOnCommandSenderResponseCallback = onCommandSenderResponseCallback;
    gOnCommandSenderErrorCallback    = onCommandSenderErrorCallback;
    gOnCommandSenderDoneCallback     = onCommandSenderDoneCallback;
}

PyChipError pychip_CommandSender_SendCommand(void * appContext, DeviceProxy * device, uint16_t timedRequestTimeoutMs,
                                             chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                                             const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs,
                                             uint16_t busyWaitMs, bool suppressResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));

    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, /* isBatchedCommands =*/false);
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
                                                   size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));
    auto remoteSessionParameters = device->GetSecureSession().Value()->GetRemoteSessionParameters();
    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(remoteSessionParameters.GetMaxPathsPerInvoke());

    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, /* isBatchedCommands =*/true);
    std::unique_ptr<CommandSender> sender =
        std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager(),
                                        /* is timed request */ timedRequestTimeoutMs != 0, suppressResponse);

    // TODO(#30986): Move away from passing these command through variadic arguments.
    va_list args;
    va_start(args, n);

    SuccessOrExit(err = sender->SetCommandSenderConfig(config));

    {
        for (size_t i = 0; i < n; i++)
        {
            void * commandPath = va_arg(args, void *);
            void * tlv         = va_arg(args, void *);
            int length         = va_arg(args, int);

            python::CommandPath invokeRequestInfoObj;
            memcpy(&invokeRequestInfoObj, commandPath, sizeof(python::CommandPath));
            const uint8_t * tlvBuffer = reinterpret_cast<const uint8_t *>(tlv);

            app::CommandPathParams cmdParams = { invokeRequestInfoObj.endpointId, /* group id */ 0, invokeRequestInfoObj.clusterId,
                                                 invokeRequestInfoObj.commandId, (app::CommandPathFlags::kEndpointIdValid) };

            CommandSender::AdditionalCommandParameters additionalParams;

            SuccessOrExit(err = sender->PrepareCommand(cmdParams, additionalParams));
            {
                auto writer = sender->GetCommandDataIBTLVWriter();
                VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
                TLV::TLVReader reader;
                reader.Init(tlvBuffer, static_cast<uint32_t>(length));
                reader.Next();
                SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(CommandDataIB::Tag::kFields), reader));
            }

            SuccessOrExit(err = sender->FinishCommand(timedRequestTimeoutMs != 0 ? Optional<uint16_t>(timedRequestTimeoutMs)
                                                                                 : Optional<uint16_t>::Missing(),
                                                      additionalParams));

            // CommandSender provides us with the CommandReference for this associated command. In order to match responses
            // we have to add CommandRef to index lookup.
            VerifyOrExit(additionalParams.mCommandRef.HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
            SuccessOrExit(err = callback->AddCommandRefToIndexLookup(additionalParams.mCommandRef.Value(), i));
        }
    }

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
    va_end(args);
    return ToPyChipError(err);
}

PyChipError pychip_CommandSender_TestOnlySendCommandTimedRequestNoTimedInvoke(
    void * appContext, DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
    const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs, uint16_t busyWaitMs, bool suppressResponse)
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));

    std::unique_ptr<CommandSenderCallback> callback =
        std::make_unique<CommandSenderCallback>(appContext, /* isBatchedCommands =*/false);
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
