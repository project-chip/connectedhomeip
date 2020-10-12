/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This header file includes common private definitions for the CHIP system layer.
 */
#pragma once

#include <system/SystemConfig.h>

//
// Common definitions for the LwIP configuration
//
#if CHIP_SYSTEM_CONFIG_USE_LWIP

#include <lwip/init.h>
#include <lwip/stats.h>

// To use LwIP 1.x, some additional definitions are required here.
#if LWIP_VERSION_MAJOR < 2

// Compatibility adaptations for statistical data in LwIP 1.x
#ifndef MEMP_STATS_GET
#define MEMP_STATS_GET(FIELD, INDEX) (lwip_stats.memp[INDEX].FIELD)
#endif // !defined(MEMP_STATS_GET)
#endif // LWIP_VERSION_MAJOR

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
