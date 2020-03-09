/*
 *
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file is an umbrella header for the Inet library, a
 *      portable Internet Protocol (IP) network interface.
 *
 */

#ifndef INET_H
#define INET_H

#include <InetLayer/InetConfig.h>

#include <InetLayer/InetError.h>
#include <InetLayer/InetInterface.h>
#include <InetLayer/InetLayerEvents.h>
#include <InetLayer/InetLayer.h>
#include <InetLayer/IPAddress.h>
#include <InetLayer/IPPrefix.h>

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#include <InetLayer/InetBuffer.h>
#include <InetLayer/InetTimer.h>
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#include <InetLayer/DNSResolver.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
#include <InetLayer/RawEndPoint.h>
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <InetLayer/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <InetLayer/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
#include <InetLayer/TunEndPoint.h>
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

#endif // !defined(INET_H)
