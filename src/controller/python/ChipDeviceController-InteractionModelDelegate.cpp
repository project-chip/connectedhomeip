#include <controller/python/ChipDeviceController-InteractionModelDelegate.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::app;

namespace chip {
namespace Controller {

PythonInteractionModelDelegate gPythonInteractionModelDelegate;

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseStatus(const CommandSender * apCommandSender,
                                                                 const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                                 const uint32_t aProtocolId, const uint16_t aProtocolCode,
                                                                 chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
                                                                 chip::CommandId aCommandId, uint8_t aCommandIndex)
{
    CommandStatus status{ aProtocolId, aProtocolCode, aEndpointId, aClusterId, aCommandId, aCommandIndex };
    if (commandResponseStatusFunct != nullptr)
    {
        commandResponseStatusFunct(reinterpret_cast<uint64_t>(apCommandSender), &status, sizeof(status));
    }
    ChipLogError(Controller, "CommandResponseStatus");
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseProtocolError(const CommandSender * apCommandSender,
                                                                        uint8_t aCommandIndex)
{
    if (commandResponseProtocolErrorFunct != nullptr)
    {
        commandResponseProtocolErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), aCommandIndex);
    }
    ChipLogError(Controller, "CommandResponseProtocolError");
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    if (commandResponseErrorFunct != nullptr)
    {
        commandResponseErrorFunct(reinterpret_cast<uint64_t>(apCommandSender), aError);
    }
    ChipLogError(Controller, "CommandResponseError: %d", aError);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonInteractionModelDelegate::CommandResponseProcessed(const app::CommandSender * apCommandSender)
{
    return this->CommandResponseError(apCommandSender, CHIP_NO_ERROR);
}

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseStatusCodeReceivedCallback(f);
}

void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseProtocolErrorCallback(f);
}

void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f)
{
    gPythonInteractionModelDelegate.SetOnCommandResponseCallback(f);
}

PythonInteractionModelDelegate & PythonInteractionModelDelegate::Instance()
{
    return gPythonInteractionModelDelegate;
}

} // namespace Controller
} // namespace chip
