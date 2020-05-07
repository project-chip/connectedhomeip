/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This provides a header for the unit testing of the CHIP ZCL
 *      Application Layer's level control cluster server
 *
 */

#ifndef UTEST_H
#define UTEST_H

#define PLATFORM_HEADER "utest.h"
#define CONFIGURATION_HEADER "utest.h"
#define CHIP_AF_API_STACK "utest.h"
#define CHIP_AF_API_ZCL_CORE "utest.h"
#define CHIP_AF_API_BUFFER_MANAGEMENT "utest.h"
#define CHIP_AF_API_EVENT_QUEUE "utest.h"
#define CHIP_AF_API_HAL "utest.h"

// User options for plugin Level Control Server
#define CHIP_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL 255
#define CHIP_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL 0

#include "chip-zcl.h"
#include "gen-types.h"
#include <stdio.h>
#endif /*  UTEST_H */
