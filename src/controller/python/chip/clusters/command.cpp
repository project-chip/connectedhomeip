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
#include <app/DeviceProxy.h>
#include <lib/support/CodeUtils.h>

#include <controller/python/chip/interaction_model/Delegate.h>
#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

using PyObject = void *;

extern "C" {
chip::ChipError::StorageType pychip_CommandSender_SendCommand(void * appContext, DeviceProxy * device,
                                                              uint16_t timedRequestTimeoutMs, chip::EndpointId endpointId,
                                                              chip::ClusterId clusterId, chip::CommandId commandId,
                                                              const uint8_t * payload, size_t length,
                                                              uint16_t interactionTimeoutMs);
}

namespace chip {
namespace python {

using OnCommandSenderResponseCallback = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                 chip::CommandId commandId,
                                                 std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                                 chip::ClusterStatus clusterStatus, const uint8_t * payload, uint32_t length);
using OnCommandSenderErrorCallback    = void (*)(PyObject appContext,
                                              std::underlying_type_t<Protocols::InteractionModel::Status> status,
                                              chip::ClusterStatus clusterStatus, uint32_t chiperror);
using OnCommandSenderDoneCallback     = void (*)(PyObject appContext);

OnCommandSenderResponseCallback gOnCommandSenderResponseCallback = nullptr;
OnCommandSenderErrorCallback gOnCommandSenderErrorCallback       = nullptr;
OnCommandSenderDoneCallback gOnCommandSenderDoneCallback         = nullptr;

class CommandSenderCallback : public CommandSender::Callback
{
public:
    CommandSenderCallback(PyObject appContext) : mAppContext(appContext) {}

    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const app::StatusIB & aStatus,
                    TLV::TLVReader * aData) override
    {
        uint8_t buffer[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
        uint32_t size = 0;
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (aData != nullptr)
        {
            // Python need to read from full TLV data the TLVReader may contain some unclean states.
            TLV::TLVWriter writer;
            writer.Init(buffer);
            CHIP_ERROR err = writer.CopyContainer(TLV::AnonymousTag(), *aData);
            if (err != CHIP_NO_ERROR)
            {
                this->OnError(apCommandSender, err);
                return;
            }
            size = writer.GetLengthWritten();
        }

        gOnCommandSenderResponseCallback(
            mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mCommandId, to_underlying(aStatus.mStatus),
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
                                      aProtocolError.IsIMStatus() ? 0 : aProtocolError.AsInteger());
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

chip::ChipError::StorageType pychip_CommandSender_SendCommand(void * appContext, DeviceProxy * device,
                                                              uint16_t timedRequestTimeoutMs, chip::EndpointId endpointId,
                                                              chip::ClusterId clusterId, chip::CommandId commandId,
                                                              const uint8_t * payload, size_t length, uint16_t interactionTimeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(device->GetSecureSession().HasValue(), CHIP_ERROR_MISSING_SECURE_SESSION.AsInteger());

    std::unique_ptr<CommandSenderCallback> callback = std::make_unique<CommandSenderCallback>(appContext);
    std::unique_ptr<CommandSender> sender           = std::make_unique<CommandSender>(callback.get(), device->GetExchangeManager(),
                                                                            /* is timed request */ timedRequestTimeoutMs != 0);

    app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, clusterId, commandId,
                                         (app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = sender->PrepareCommand(cmdParams, false));

    {
        auto writer = sender->GetCommandDataIBTLVWriter();
        TLV::TLVReader reader;
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(payload, length);
        reader.Next();
        SuccessOrExit(writer->CopyContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kFields)), reader));
    }

    SuccessOrExit(err = sender->FinishCommand(timedRequestTimeoutMs != 0 ? Optional<uint16_t>(timedRequestTimeoutMs)
                                                                         : Optional<uint16_t>::Missing()));

    SuccessOrExit(err = sender->SendCommandRequest(device->GetSecureSession().Value(),
                                                   interactionTimeoutMs != 0
                                                       ? MakeOptional(System::Clock::Milliseconds32(interactionTimeoutMs))
                                                       : Optional<System::Clock::Timeout>::Missing()));

    sender.release();
    callback.release();

exit:
    return err.AsInteger();
}
}
