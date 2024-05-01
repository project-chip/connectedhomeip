/*
 *  Copyright 2022-2024 NXP
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

namespace chip {
namespace NXP {
namespace App {
/**
 * API allowing to register matter cli command
 */
CHIP_ERROR AppMatterCli_RegisterCommands(void);
/**
 * API allowing to start matter cli task
 */
CHIP_ERROR AppMatterCli_StartTask(void);

} // namespace App
} // namespace NXP
} // namespace chip

#ifdef __cplusplus
}
#endif

#endif /* _MATTER_CLI_H_ */
