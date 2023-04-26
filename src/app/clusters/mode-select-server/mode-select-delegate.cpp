#include <app/clusters/mode-select-server/mode-select-server.h>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;

void Delegate::HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response)
{
    response.status = uint8_t(ChangeToModeResponseStatus::kSuccess);
    // todo set text to null
}

Status Delegate::IsSupportedMode(uint8_t modeValue) 
{ 
    for (ModeOptionStructType modeStruct : modeOptions)
    {
        if (modeStruct.mode == modeValue)
        {
            return Status::Success;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %u", modeValue);
    return Status::InvalidCommand;
}

Status Delegate::GetMode(uint8_t modeValue, ModeOptionStructType &modeOption)
{
    for (ModeOptionStructType modeStruct : modeOptions)
    {
        if (modeStruct.mode == modeValue)
        {
            modeOption = modeStruct;
            return Status::Success;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %u", modeValue);
    return Status::InvalidCommand;
}