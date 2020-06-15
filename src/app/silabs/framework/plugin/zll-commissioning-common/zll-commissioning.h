/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Definitions for the ZLL Commissioning Common plugin.
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * zll-commissioning.h
// *
// *
// Copyright 2010-2018 Silicon Laboratories, Inc.
// *******************************************************************

// This is the master header file for the zll-profile API. Pull in the individual
// plugin files for the application.
#include "zll-commissioning-common.h"
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
#include "app/framework/plugin/zll-commissioning-client/zll-commissioning-client.h"
#endif
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
#include "app/framework/plugin/zll-commissioning-server/zll-commissioning-server.h"
#endif
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK
#include "app/framework/plugin/zll-commissioning-network/zll-commissioning-network.h"
#endif
