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

#include "TestInetCommonOptions.h"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <inet/InetFaultInjection.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <system/SystemFaultInjection.h>

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Inet;

// Global Variables

NetworkOptions gNetworkOptions;
FaultInjectionOptions gFaultInjectionOptions;

NetworkOptions::NetworkOptions()
{
    static OptionDef optionDefs[] = {
        { "local-addr", kArgumentRequired, 'a' },
        { "node-addr", kArgumentRequired, kToolCommonOpt_NodeAddr }, /* alias for local-addr */
#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        { "tap-device", kArgumentRequired, kToolCommonOpt_TapDevice },
        { "ipv4-gateway", kArgumentRequired, kToolCommonOpt_IPv4GatewayAddr },
        { "ipv6-gateway", kArgumentRequired, kToolCommonOpt_IPv6GatewayAddr },
        { "dns-server", kArgumentRequired, 'X' },
        { "debug-lwip", kNoArgument, kToolCommonOpt_DebugLwIP },
        { "event-delay", kArgumentRequired, kToolCommonOpt_EventDelay },
        { "tap-system-config", kNoArgument, kToolCommonOpt_TapInterfaceConfig },
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        {}
    };
    OptionDefs = optionDefs;

    HelpGroupName = "NETWORK OPTIONS";

    OptionHelp = "  -a, --local-addr, --node-addr <ip-addr>\n"
                 "       Local address for the node.\n"
                 "\n"
#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
                 "  --tap-device <tap-dev-name>\n"
                 "       TAP device name for LwIP hosted OS usage. Defaults to chip-dev-<node-id>.\n"
                 "\n"
                 "  --ipv4-gateway <ip-addr>\n"
                 "       Address of default IPv4 gateway.\n"
                 "\n"
                 "  --ipv6-gateway <ip-addr>\n"
                 "       Address of default IPv6 gateway.\n"
                 "\n"
                 "  -X, --dns-server <ip-addr>\n"
                 "       IPv4 address of local DNS server.\n"
                 "\n"
                 "  --debug-lwip\n"
                 "       Enable LwIP debug messages.\n"
                 "\n"
                 "  --event-delay <int>\n"
                 "       Delay event processing by specified number of iterations. Defaults to 0.\n"
                 "\n"
                 "  --tap-system-config\n"
                 "       Use configuration on each of the Linux TAP interfaces to configure LwIP's interfaces.\n"
                 "\n"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        ;

    // Defaults.
    LocalIPv4Addr.clear();
    LocalIPv6Addr.clear();

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    TapDeviceName.clear();
    LwIPDebugFlags = 0;
    EventDelay     = 0;
    IPv4GatewayAddr.clear();
    IPv6GatewayAddr.clear();
    DNSServerAddr      = Inet::IPAddress::Any;
    TapUseSystemConfig = false;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
}

bool NetworkOptions::HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    Inet::IPAddress localAddr;

    switch (id)
    {
    case 'a':
    case kToolCommonOpt_NodeAddr:
        if (!ParseIPAddress(arg, localAddr))
        {
            PrintArgError("%s: Invalid value specified for local IP address: %s\n", progName, arg);
            return false;
        }
#if INET_CONFIG_ENABLE_IPV4
        if (localAddr.IsIPv4())
        {
            LocalIPv4Addr.push_back(localAddr);
        }
        else
        {
            LocalIPv6Addr.push_back(localAddr);
        }
#else  // INET_CONFIG_ENABLE_IPV4
        LocalIPv6Addr.push_back(localAddr);
#endif // INET_CONFIG_ENABLE_IPV4
        break;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    case 'X':
        if (!ParseIPAddress(arg, DNSServerAddr))
        {
            PrintArgError("%s: Invalid value specified for DNS server address: %s\n", progName, arg);
            return false;
        }
        break;
    case kToolCommonOpt_TapDevice:
        TapDeviceName.push_back(arg);
        break;

    case kToolCommonOpt_IPv4GatewayAddr: {
        if (!ParseIPAddress(arg, localAddr) || !localAddr.IsIPv4())
        {
            PrintArgError("%s: Invalid value specified for IPv4 gateway address: %s\n", progName, arg);
            return false;
        }
        IPv4GatewayAddr.push_back(localAddr);
    }
    break;

    case kToolCommonOpt_IPv6GatewayAddr: {
        if (!ParseIPAddress(arg, localAddr))
        {
            PrintArgError("%s: Invalid value specified for IPv6 gateway address: %s\n", progName, arg);
            return false;
        }
        IPv6GatewayAddr.push_back(localAddr);
    }
    break;

    case kToolCommonOpt_DebugLwIP:
#if defined(LWIP_DEBUG)
#if CHIP_TARGET_STYLE_UNIX
        gLwIP_DebugFlags = (LWIP_DBG_ON | LWIP_DBG_TRACE | LWIP_DBG_STATE | LWIP_DBG_FRESH | LWIP_DBG_HALT);
#endif
#endif
        break;
    case kToolCommonOpt_EventDelay:
        if (!ParseInt(arg, EventDelay))
        {
            PrintArgError("%s: Invalid value specified for event delay: %s\n", progName, arg);
            return false;
        }
        break;

    case kToolCommonOpt_TapInterfaceConfig:
        TapUseSystemConfig = true;
        break;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

FaultInjectionOptions::FaultInjectionOptions()
{
    static OptionDef optionDefs[] = { { "faults", kArgumentRequired, kToolCommonOpt_FaultInjection },
                                      { "iterations", kArgumentRequired, kToolCommonOpt_FaultTestIterations },
                                      { "debug-resource-usage", kNoArgument, kToolCommonOpt_DebugResourceUsage },
                                      { "print-fault-counters", kNoArgument, kToolCommonOpt_PrintFaultCounters },
                                      { "extra-cleanup-time", kArgumentRequired, kToolCommonOpt_ExtraCleanupTime },
                                      {} };
    OptionDefs                    = optionDefs;

    HelpGroupName = "FAULT INJECTION OPTIONS";

    OptionHelp = "  --faults <fault-string>\n"
                 "       Inject specified fault(s) into the operation of the tool at runtime.\n"
                 "\n"
                 "  --iterations <int>\n"
                 "       Execute the program operation the given number of times\n"
                 "\n"
                 "  --debug-resource-usage\n"
                 "       Print all stats counters before exiting.\n"
                 "\n"
                 "  --print-fault-counters\n"
                 "       Print the fault-injection counters before exiting.\n"
                 "\n"
                 "  --extra-cleanup-time\n"
                 "       Allow extra time before asserting resource leaks; this is useful when\n"
                 "       running fault-injection tests to let the system free stale ExchangeContext\n"
                 "       instances after RMP has exhausted all retransmission; a failed RMP transmission\n"
                 "       should fail a normal happy-sequence test, but not necessarily a fault-injection test.\n"
                 "       The value is in milliseconds; a common value is 10000.\n"
                 "\n"
                 "";

    // Defaults
    TestIterations       = 1;
    DebugResourceUsage   = false;
    PrintFaultCounters   = false;
    ExtraCleanupTimeMsec = 0;
}

#if defined(CHIP_WITH_NLFAULTINJECTION) && CHIP_WITH_NLFAULTINJECTION
bool FaultInjectionOptions::HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    using namespace nl::FaultInjection;

    GetManagerFn faultMgrFnTable[] = { Inet::FaultInjection::GetManager, System::FaultInjection::GetManager };
    size_t faultMgrFnTableLen      = sizeof(faultMgrFnTable) / sizeof(faultMgrFnTable[0]);

    switch (id)
    {
    case kToolCommonOpt_FaultInjection: {
        chip::Platform::ScopedMemoryString mutableArg(arg, strlen(arg));
        assert(mutableArg);
        bool parseRes = ParseFaultInjectionStr(mutableArg.Get(), faultMgrFnTable, faultMgrFnTableLen);
        if (!parseRes)
        {
            PrintArgError("%s: Invalid string specified for fault injection option: %s\n", progName, arg);
            return false;
        }
        break;
    }
    case kToolCommonOpt_FaultTestIterations:
        if ((!ParseInt(arg, TestIterations)) || (TestIterations == 0))
        {
            PrintArgError("%s: Invalid value specified for the number of iterations to execute: %s\n", progName, arg);
            return false;
        }
        break;
    case kToolCommonOpt_DebugResourceUsage:
        DebugResourceUsage = true;
        break;
    case kToolCommonOpt_PrintFaultCounters:
        PrintFaultCounters = true;
        break;
    case kToolCommonOpt_ExtraCleanupTime:
        if ((!ParseInt(arg, ExtraCleanupTimeMsec)) || (ExtraCleanupTimeMsec == 0))
        {
            PrintArgError("%s: Invalid value specified for the extra time to wait before checking for leaks: %s\n", progName, arg);
            return false;
        }
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}
#endif // defined(CHIP_WITH_NLFAULTINJECTION) && CHIP_WITH_NLFAULTINJECTION
