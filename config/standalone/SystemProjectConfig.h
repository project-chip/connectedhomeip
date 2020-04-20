/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *      chip::System project configuration for standalone builds on Linux and OS X.
 *
 */
#ifndef SYSTEMPROJECTCONFIG_H
#define SYSTEMPROJECTCONFIG_H

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
// Uncomment this for larger buffers (e.g. to support a bigger CHIP_CONFIG_TUNNEL_INTERFACE_MTU).
//#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 9050
#endif

#endif /* SYSTEMPROJECTCONFIG_H */
