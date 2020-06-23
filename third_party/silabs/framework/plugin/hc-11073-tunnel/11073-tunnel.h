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
 * @brief APIs and defines for the HC 11073 Tunnel plugin, which implements the
 *        tunneling of 11073 data over zigbee.
 *******************************************************************************
   ******************************************************************************/

// These are statically defined by the spec. Defines provided here
// to improve plugin readability.
#define CLUSTER_ID_11073_TUNNEL 0x0614
#define ATTRIBUTE_11073_TUNNEL_MANAGER_TARGET 0x0001
#define ATTRIBUTE_11073_TUNNEL_MANAGER_ENDPOINT 0x0002
#define ATTRIBUTE_11073_TUNNEL_CONNECTED 0x0003
#define ATTRIBUTE_11073_TUNNEL_PREEMPTIBLE 0x0004
#define ATTRIBUTE_11073_TUNNEL_IDLE_TIMEOUT 0x0005

// These are variable and should be defined by the application using
// this plugin.
#ifndef HC_11073_TUNNEL_ENDPOINT
  #define HC_11073_TUNNEL_ENDPOINT 1
#endif
