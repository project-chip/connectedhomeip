#include "OTASoftwareUpdateInteractive.h"

void registerClusterOtaSoftwareUpdateProviderInteractive(Commands & commands)
{

    const char * clusterName = "OtaSoftwareUpdateApp";

    commands_list clusterCommands = {
        make_unique<OTASoftwareUpdateSetFilePath>(), //
        make_unique<OTASoftwareUpdateSetParams>(),   //
    };

    commands.Register(clusterName, clusterCommands);
}
