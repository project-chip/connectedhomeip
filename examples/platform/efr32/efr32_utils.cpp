/*
 * SPDX-FileCopyrightText: (c) 2022 Silabs.
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "efr32_utils.h"
#include "init_efrPlatform.h"
#include "sl_system_kernel.h"

#include <matter_config.h>

void appError(int err)
{
    EFR32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}
