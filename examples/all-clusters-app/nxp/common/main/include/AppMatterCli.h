/*
 *  Copyright 2022 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MATTER_CLI_H_
#define _MATTER_CLI_H_

#include <lib/core/CHIPError.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * API allowing to register matter cli command
 */
CHIP_ERROR AppMatterCli_RegisterCommands(void);

#ifdef __cplusplus
}
#endif

#endif /* _MATTER_CLI_H_ */
