/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          This file contains matter_pal.
 */

#ifndef _MATTER_PAL_H_
#define _MATTER_PAL_H_

#include <os/mem.h>
#include <os/os.h>

#include <components/system.h>

#include <modules/wifi.h>
#include <modules/wifi_types.h>

#include "common/bk_err.h"
#include "common/bk_include.h"

#if CONFIG_FLASH_ORIGIN_API
#include "BkDriverFlash.h"
#else
#include "driver/flash.h"
#include <driver/flash_partition.h>
#endif

#include "flash_namespace_value.h"

#include "wlan_ui_pub.h"

#include "bk_wifi.h"
#include "bk_wifi_types.h"
#include "bk_wifi_v1.h"
#include "net.h"

#include "at_ble_common.h"
#include "ble.h"

#endif // _MATTER_PAL_H_
// eof
