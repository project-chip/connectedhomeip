/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ethernetInterface_init(void);
void ethernetInterface_eventGotIP(struct netif * interface);
struct netif * deviceInterface_getNetif(void);

void network_netif_ext_callback(struct netif * netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args);

#ifdef __cplusplus
}
#endif
