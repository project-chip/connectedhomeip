#include <app/InteractionModelDelegate.h>

namespace chip {
namespace Controller {

struct __attribute__((packed)) CommandStatus
{
    uint32_t ProtocolId;
    uint16_t ProtocolCode;
    chip::EndpointId EndpointId;
    chip::ClusterId ClusterId;
    chip::CommandId CommandId;
    uint8_t CommandIndex;
};

extern "C" {
typedef void (*PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct)(uint64_t commandSenderPtr,
                                                                                        void * commandStatusBuf,
                                                                                        uint32_t commandStatusBufLen);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct)(uint64_t commandSenderPtr, uint8_t commandIndex);
typedef void (*PythonInteractionModelDelegate_OnCommandResponseFunct)(uint64_t commandSenderPtr, uint32_t error);

void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f);
void pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f);
}

class PythonInteractionModelDelegate : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR CommandResponseStatus(const app::CommandSender * apCommandSender,
                                     const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const uint32_t aProtocolId,
                                     const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
                                     chip::CommandId aCommandId, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseProtocolError(const app::CommandSender * apCommandSender, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    CHIP_ERROR CommandResponseProcessed(const app::CommandSender * apCommandSender) override;

    static PythonInteractionModelDelegate & Instance();

private:
    friend void pychip_InteractionModelDelegate_SetCommandResponseStatusCallback(
        PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f);
    friend void pychip_InteractionModelDelegate_SetCommandResponseProtocolErrorCallback(
        PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f);
    friend void
    pychip_InteractionModelDelegate_SetCommandResponseErrorCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f);

    PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct commandResponseStatusFunct   = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct commandResponseProtocolErrorFunct = nullptr;
    PythonInteractionModelDelegate_OnCommandResponseFunct commandResponseErrorFunct                      = nullptr;

    void SetOnCommandResponseStatusCodeReceivedCallback(PythonInteractionModelDelegate_OnCommandResponseStatusCodeReceivedFunct f)
    {
        commandResponseStatusFunct = f;
    }

    void SetOnCommandResponseProtocolErrorCallback(PythonInteractionModelDelegate_OnCommandResponseProtocolErrorFunct f)
    {
        commandResponseProtocolErrorFunct = f;
    }

    void SetOnCommandResponseCallback(PythonInteractionModelDelegate_OnCommandResponseFunct f) { commandResponseErrorFunct = f; }
};

} // namespace Controller
} // namespace chip
