/*
 * Copyright (c) 2023 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#define MCUBOOT_DOWNGRADE_PREVENTION
#define MCUBOOT_HAVE_LOGGING 1
#define MCUBOOT_IMAGE_NUMBER 1
#define MCUBOOT_MAX_IMG_SECTORS 492
#define MCUBOOT_OVERWRITE_ONLY
#define MCUBOOT_SIGN_EC256
#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS
#define MCUBOOT_USE_TI_CRYPTO
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
#define MCUBOOT_WATCHDOG_FEED()                                                                                                    \
    {}

#define TI_BOOT_USE_EXTERNAL_FLASH

#ifdef MCUBOOT_DATA_SHARING
#define MCUBOOT_SHARED_DATA_BASE 0x20000000
#define MCUBOOT_SHARED_DATA_SIZE 0x400
#endif

#if defined(TI_BOOT_USE_EXTERNAL_FLASH) && (!defined(MCUBOOT_OVERWRITE_ONLY) || defined(MCUBOOT_DIRECT_XIP))
#error "If external flash is configured, only MCUBOOT_OVERWRITE_ONLY is supported"
#endif
