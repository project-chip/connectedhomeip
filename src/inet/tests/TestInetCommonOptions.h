/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Common command-line option handling code for test applications.
 *
 */

#pragma once

#include <vector>

#include <inet/InetArgParser.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPArgParser.hpp>
#include <system/SystemLayer.h>

#define TOOL_OPTIONS_ENV_VAR_NAME "CHIP_INET_TEST_OPTIONS"

enum
{
    kToolCommonOpt_NodeAddr = 1000,
    kToolCommonOpt_EventDelay,
    kToolCommonOpt_FaultInjection,
    kToolCommonOpt_FaultTestIterations,
    kToolCommonOpt_DebugResourceUsage,
    kToolCommonOpt_PrintFaultCounters,
    kToolCommonOpt_ExtraCleanupTime,
    kToolCommonOpt_DebugLwIP,
    kToolCommonOpt_IPv4GatewayAddr,
    kToolCommonOpt_IPv6GatewayAddr,
    kToolCommonOpt_TapDevice,
    kToolCommonOpt_TapInterfaceConfig,
};

/**
 * Handler for options that control local network/network interface configuration.
 */
class NetworkOptions : public chip::ArgParser::OptionSetBase
{
public:
    std::vector<chip::Inet::IPAddress> LocalIPv4Addr;
    std::vector<chip::Inet::IPAddress> LocalIPv6Addr;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    std::vector<chip::Inet::IPAddress> IPv4GatewayAddr;
    std::vector<chip::Inet::IPAddress> IPv6GatewayAddr;
    chip::Inet::IPAddress DNSServerAddr;
    std::vector<const char *> TapDeviceName;
    uint8_t LwIPDebugFlags;
    uint32_t EventDelay;
    bool TapUseSystemConfig;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    NetworkOptions();

    bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg) override;
};

extern NetworkOptions gNetworkOptions;

/**
 * Handler for options that control fault injection testing behavior.
 */
class FaultInjectionOptions : public chip::ArgParser::OptionSetBase
{
public:
    uint32_t TestIterations;
    bool DebugResourceUsage;
    bool PrintFaultCounters;
    uint32_t ExtraCleanupTimeMsec;

    FaultInjectionOptions();

    bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg) override;
};

extern FaultInjectionOptions gFaultInjectionOptions;
