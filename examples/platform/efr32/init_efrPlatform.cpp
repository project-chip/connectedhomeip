/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppConfig.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include <string.h>

#include <mbedtls/platform.h>

#if CHIP_ENABLE_OPENTHREAD
#include <openthread-core-config.h>
#include <openthread/cli.h>
#include <openthread/config.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <utils/uart.h>

#include "platform-efr32.h"

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#include "openthread/heap.h"
#endif // OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#endif // CHIP_ENABLE_OPENTHREAD

#include "init_efrPlatform.h"
#include "sl_component_catalog.h"
#include "sl_mbedtls.h"
#include "sl_system_init.h"

void initAntenna(void);

void init_efrPlatform(void)
{
    sl_system_init();
    sl_mbedtls_init();

#if EFR32_LOG_ENABLED
    efr32InitLog();
#endif

#if CHIP_ENABLE_OPENTHREAD
    efr32RadioInit();
    efr32AlarmInit();
#endif // CHIP_ENABLE_OPENTHREAD
}

#ifdef __cplusplus
}
#endif
