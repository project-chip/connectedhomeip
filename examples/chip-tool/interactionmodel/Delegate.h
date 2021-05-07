#pragma once

#include <app/InteractionModelDelegate.h>
#include <commands/common/Command.h>

class ChipToolInteractionModelDelegate : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR CommandResponseStatus(const chip::app::CommandSender * apCommandSender,
                                     const chip::Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                     const uint32_t aProtocolId, const uint16_t aProtocolCode, chip::EndpointId aEndpointId,
                                     const chip::ClusterId aClusterId, chip::CommandId aCommandId, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    CHIP_ERROR CommandResponseProcessed(const chip::app::CommandSender * apCommandSender) override;

    void SetCommandObject(Command * command) { mpCommand = command; }

private:
    Command * mpCommand;
};
