/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
