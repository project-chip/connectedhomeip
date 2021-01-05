/*
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

#pragma once

#include <app/server/AppDelegate.h>
#include <inet/InetConfig.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#if INET_CONFIG_ENABLE_IPV4
using DemoTransportMgr = chip::TransportMgr<chip::Transport::UDP, chip::Transport::UDP>;
#else
using DemoTransportMgr = chip::TransportMgr<chip::Transport::UDP>;
#endif

/**
 * Initialize DataModelHandler and start CHIP datamodel server, the server
 * assumes the platform's networking has been setup already.
 *
 * @param [in] delegate   An optional AppDelegate
 */
void InitServer(AppDelegate * delegate = nullptr);
