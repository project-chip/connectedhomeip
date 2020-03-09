/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Header file for the obsolescent InetBuffer type definition, which
 *      provides transitional definitions that source-code compatible with
 *      implementations that rely on the legacy InetBuffer class.
 */

#ifndef INETBUFFER_H
#define INETBUFFER_H

#include <InetLayer/InetConfig.h>

#if !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#error "#include <InetLayer/InetBuffer.h> // while !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES."

#else // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#include <SystemLayer/SystemPacketBuffer.h>

namespace nl {
namespace Inet {

typedef Weave::System::PacketBuffer InetBuffer;

#if !WEAVE_SYSTEM_CONFIG_USE_LWIP
struct pbuf : public Weave::System::pbuf { };
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP

} // namespace Inet
} // namespace nl

#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#endif // !defined(INETBUFFER_H)
