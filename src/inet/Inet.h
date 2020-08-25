/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#include <inet/InetConfig.h>

#include <inet/IPAddress.h>
#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <inet/InetLayerEvents.h>

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#include <inet/DNSResolver.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
#include <inet/RawEndPoint.h>
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <inet/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <inet/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#endif // !defined(INET_H)
