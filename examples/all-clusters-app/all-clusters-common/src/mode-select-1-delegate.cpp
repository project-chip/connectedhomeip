#include <app/util/config.h>
#include <mode-select-1-delegate.h>

using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

CHIP_ERROR ModeSelectDelegate1::Init()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Called ModeSelectDelegate1::Init()!!");
    return CHIP_NO_ERROR;
}
