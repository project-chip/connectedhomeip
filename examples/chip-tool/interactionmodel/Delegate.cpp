#include "Delegate.h"

#include <app/util/af-enums.h>
#include <gen/CHIPClientCallbacks.h>

using namespace chip;

CHIP_ERROR ChipToolInteractionModelDelegate::CommandResponseStatus(
    const chip::app::CommandSender * apCommandSender, const chip::Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
    const uint32_t aProtocolId, const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
    chip::CommandId aCommandId, uint8_t aCommandIndex)
{
    IMDefaultResponseCallback(apCommandSender,
                              (aGeneralCode == chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess && aProtocolCode == 0)
                                  ? EMBER_ZCL_STATUS_SUCCESS
                                  : EMBER_ZCL_STATUS_FAILURE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipToolInteractionModelDelegate::CommandResponseError(const chip::app::CommandSender * apCommandSender,
                                                                  CHIP_ERROR aError)
{
    ChipLogError(Zcl, "Failed to handle command response for transaction %p: %s", apCommandSender, ErrorStr(aError));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipToolInteractionModelDelegate::CommandResponseProcessed(const chip::app::CommandSender * apCommandSender)
{
    VerifyOrReturnError(mpCommand != nullptr, CHIP_NO_ERROR);
    mpCommand->UpdateWaitForResponse(false);

    return CHIP_NO_ERROR;
}
