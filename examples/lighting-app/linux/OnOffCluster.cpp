
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/MessageDef.h>
#include <app/util/basic-types.h>
#include <app/im-encoder.h>

#include <support/logging/CHIPLogging.h>

#include "LightingManager.h"

namespace chip {
namespace app {
namespace cluster {
namespace OnOff {

void HandleOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    LightingMgr().InitiateAction(LightingManager::OFF_ACTION);
    EncodeOffCommand(apCommandObj, 1, 0);
}

void HandleOnCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    LightingMgr().InitiateAction(LightingManager::ON_ACTION);
    EncodeOnCommand(apCommandObj, 1, 0);
}

void HandleToggleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    LightingMgr().InitiateAction(LightingMgr().IsTurnedOn() ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION);
    EncodeToggleCommand(apCommandObj, 1, 0);
}

} // namespace OnOff
} // namespace cluster
} // namespace app
} // namespace chip
