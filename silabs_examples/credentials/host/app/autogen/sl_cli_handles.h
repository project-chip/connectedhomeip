#ifndef SL_CLI_HANDLES_H
#define SL_CLI_HANDLES_H

#include "sl_cli.h"

#ifdef __cplusplus
extern "C" {
#endif

// CLI instances present in the project.
#define SL_CLI_EXAMPLE_PRESENT


// CLI handles.
extern sl_cli_handle_t sl_cli_example_handle;


// CLI command groups.
extern sl_cli_command_group_t sl_cli_example_command_group;


extern const sl_cli_handle_t sl_cli_handles[];
extern const uint32_t sl_cli_handles_count;

#ifdef __cplusplus
}
#endif

#endif // ifndef SL_CLI_HANDLES_H