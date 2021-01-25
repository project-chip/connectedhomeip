
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/MessageDef/MessageDef.h>
#include <app/im-encoder.h>
#include <app/util/basic-types.h>

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace cluster {
namespace OnOff {

// TODO: Call handler in on-off.cpp here.
void HandleOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    EncodeOffCommand(apCommandObj, 1, 0);
}

void HandleOnCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    EncodeOnCommand(apCommandObj, 1, 0);
}

void HandleToggleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    EncodeToggleCommand(apCommandObj, 1, 0);
}

} // namespace OnOff
} // namespace cluster
} // namespace app
} // namespace chip
