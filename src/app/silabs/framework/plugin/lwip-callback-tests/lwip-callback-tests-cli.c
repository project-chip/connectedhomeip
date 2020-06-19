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
 * @brief This file provides CLI commands to manipulate the network interface.
 *******************************************************************************
   ******************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "rtos/rtos.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/err.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "app/framework/plugin/lwip/lwip-main.h"

//=============================================================================
// Globals
extern void clearStateCallbackRegistrations(void);
extern bool emberAfPluginLwipStateRegisterCallback(netif_status_callback_fn handler, const uint16_t handlerID);
extern struct netif* emberAfPluginLwipGetNetIf(void);

void testCallback1(struct netif* n)
{
  emberAfCorePrintln("in testCallback 1");
}

void testCallback2(struct netif* n)
{
  emberAfCorePrintln("in testCallback 2");
}
void testCallback3(struct netif* n)
{
  emberAfCorePrintln("in testCallback 3");
}
void testCallback4(struct netif* n)
{
  emberAfCorePrintln("in testCallback 4");
}

void emberAfPluginLwipCallbackTestsReset(void)
{
  clearStateCallbackRegistrations();
}

void emberAfPluginLwipCallbackTestsCB1(void)
{
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback1, 0xEEE1);
}

void emberAfPluginLwipCallbackTestsCB2(void)
{
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback2, 0xEEE2);
}

void emberAfPluginLwipCallbackTestsCB3(void)
{
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback3, 0xEEE3);
}

void emberAfPluginLwipCallbackTestsCB4(void)
{
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback4, 0xEEE4);
}

void emberAfPluginLwipCallbackTestsbad(void)
{
  // Invalid IDs
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback4, 0xFFFF);
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback4, 0x0000);
  emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)testCallback4, 0x0001);

  //Invalid callback
  emberAfPluginLwipStateRegisterCallback(NULL, 0xBCBC);
}
#endif DOXYGEN_SHOULD_SKIP_THIS
