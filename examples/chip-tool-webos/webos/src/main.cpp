#if !CHIP_TOOL_MODE
#include <luna-service2/lunaservice.h>
#include <PmLogLib.h>

#include "mattermanagerservice.h"
#include "logging.h"

PmLogContext logContext;
static const char* const logContextName = "chip-service";
static GMainLoop *mainLoop = NULL;

#else
#include "commands/common/Commands.h"

#include "commands/discover/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"

#include <zap-generated/cluster/Commands.h>
#include <zap-generated/reporting/Commands.h>
#include <zap-generated/test/Commands.h>
#endif // CHIP_TOOL_MODE

int main(int argc, char **argv)
{
#if !CHIP_TOOL_MODE
	PmLogErr error = PmLogGetContext(logContextName, &logContext);
	if (error != kPmLogErr_None)
	{
		fprintf(stderr, "Failed to setup up log context %s\n", logContextName);
		abort();
	}

	mainLoop = g_main_loop_new(NULL, FALSE);

	MATTER_DEBUG("Starting chip-tool service");

	MatterManagerService::GetInstance()->attachToLoop(mainLoop);
	MatterManagerService::GetInstance()->setMainLoop(mainLoop);
	g_main_loop_run(mainLoop);

	g_main_loop_unref(mainLoop);

	return 0;
#else
    Commands commands;
    registerCommandsDiscover(commands);
    registerCommandsPayload(commands);
    registerCommandsPairing(commands);
    registerCommandsReporting(commands);
    registerCommandsTests(commands);
    registerClusters(commands);

    return commands.Run(argc, argv);
#endif // CHIP_TOOL_MODE

}
