#include <app/util/config.h>
#include <mode-select-1-delegate.h>
#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

CHIP_ERROR ModeSelectDelegate1::Init()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Called ModeSelectDelegate1::Init()!!");
    return CHIP_NO_ERROR;
}

void ModeSelectDelegate1::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
{
    // todo replace this accessor function
    uint8_t currentMode;
    EmberAfStatus status = Attributes::CurrentMode::Get(0x1, &currentMode);

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
        response.status = int8_t(ChangeToModeResponseStatus::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
        return;
    }

    if (currentMode == 0 && mode == 7) {
        response.status = int8_t(ChangeToModeResponseStatus::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Cannot change to mode 7 from mode 0", 35));
        return;
    }
}