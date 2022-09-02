#ifndef SL_CLI_INSTANCES_H
#define SL_CLI_INSTANCES_H

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#include "sl_cli.h"

#ifdef __cplusplus
extern "C" {
#endif

void sl_cli_instances_init(void);
#if !defined(SL_CATALOG_KERNEL_PRESENT)
bool sl_cli_instances_is_ok_to_sleep(void);
void sl_cli_instances_tick(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_CLI_INSTANCES_H