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

#include <app/CommandSender.h>
#include <controller/CHIPDevice.h>
#include <lib/support/CodeUtils.h>

#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

using PyObject = void *;

extern "C" {
chip::ChipError::StorageType pychip_CommandSender_SendCommand(void * appContext, Controller::Device * device,
                                                              chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                              chip::CommandId commandId, const uint8_t * payload, size_t length);
}

namespace chip {
namespace python {

using OnCommandSenderResponseCallback = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                 chip::CommandId commandId, const uint8_t * payload, uint32_t length);
using OnCommandSenderErrorCallback    = void (*)(PyObject appContext,
                                              std::underlying_type_t<Protocols::InteractionModel::Status> imstatus,
                                              uint32_t chiperror);
using OnCommandSenderDoneCallback     = void (*)(PyObject appContext);

OnCommandSenderResponseCallback gOnCommandSenderResponseCallback = nullptr;
OnCommandSenderErrorCallback gOnCommandSenderErrorCallback       = nullptr;
OnCommandSenderDoneCallback gOnCommandSenderDoneCallback         = nullptr;

class CommandSenderCallback : public CommandSender::Callback
{
public:
    CommandSenderCallback(PyObject appContext) : mAppContext(appContext) {}

    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, TLV::TLVReader * aData) override
    {
        const uint8_t * buffer = nullptr;
        uint32_t size          = 0;
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (aData != nullptr)
        {
            buffer = aData->GetReadPoint();
            size   = aData->GetRemainingLength();
        }

        gOnCommandSenderResponseCallback(mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mCommandId, buffer, size);
    }

    void OnError(const CommandSender * apCommandSender, Protocols::InteractionModel::Status aInteractionModelStatus,
                 CHIP_ERROR aProtocolError) override
    {
        gOnCommandSenderErrorCallback(mAppContext, to_underlying(aInteractionModelStatus), aProtocolError.AsInteger());
    }

    void OnDone(CommandSender * apCommandSender) override
    {
        gOnCommandSenderDoneCallback(mAppContext);
        delete apCommandSender;
        delete this;
    };

private:
    PyObject mAppContext = nullptr;
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

chip::ChipError::StorageType pychip_CommandSender_SendCommand(void * appContext, Controller::Device * device,
                                                              chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                              chip::CommandId commandId, const uint8_t * payload, size_t length)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<CommandSenderCallback> callback = std::make_unique<CommandSenderCallback>(appContext);
    std::unique_ptr<CommandSender> sender           = std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager());

    app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, clusterId, commandId,
                                         (app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = sender->PrepareCommand(cmdParams));

    {
        auto writer = sender->GetCommandDataElementTLVWriter();
        TLV::TLVReader reader;
        TLV::TLVType type;
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(payload, length);
        reader.Next();
        reader.EnterContainer(type);
        while (reader.Next() == CHIP_NO_ERROR)
        {
            TLV::TLVReader tReader;
            tReader.Init(reader);
            writer->CopyElement(tReader);
        }
    }

    SuccessOrExit(err = sender->FinishCommand());
    SuccessOrExit(err = device->SendCommands(sender.get()));

    sender.release();
    callback.release();

exit:
    return err.AsInteger();
}
}
