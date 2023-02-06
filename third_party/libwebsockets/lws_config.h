/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#define LWS_LIBRARY_VERSION "4.3.99"

//
// Major individual features
//
#define LWS_WITH_NETWORK // "Compile with network-related code"  - default: ON
#define LWS_ROLE_H1      // "Compile with support for http/1 (needed for ws)"  - default: ON
#define LWS_ROLE_WS      // "Compile with support for websockets"  - default: ON
#define LWS_WITH_IPV6    // "Compile with support for ipv6" - default: OFF
#define LWS_UNIX_SOCK    // "Compile with support for UNIX domain socket if OS supports it"  - default: ON

//
// Client / Server / Test Apps build control
//
#define LWS_WITHOUT_CLIENT // "Don't build the client part of the library" default - OFF

//
// Extensions (permessage-deflate)
//
#define LWS_WITHOUT_EXTENSIONS // "Don't compile with extensions" - default: ON

//
// Helpers + misc
//
#define LWS_WITHOUT_DAEMONIZE // "Don't build the daemonization api" - default: ON
#define LWS_LOGS_TIMESTAMP    // "Timestamp at start of logs" - default: ON
#define LWS_LOG_TAG_LIFECYCLE // "Log tagged object lifecycle as NOTICE" - default: ON
#define LWS_WITH_NO_LOGS      // "Disable all logging other than _err and _user from being compiled in" - defaults: OFF

//
// Implied Options
//
#define LWS_HAS_INTPTR_T
#define LWS_WITH_POLL
#define LWS_MAX_SMP 1

#ifdef LWS_WITHOUT_DAEMONIZE
#define LWS_NO_DAEMONIZE
#endif

#ifdef LWS_WITH_HTTP2
#define LWS_ROLE_H2
#endif

#ifndef LWS_WITHOUT_SERVER
#define LWS_WITH_SERVER
#endif
