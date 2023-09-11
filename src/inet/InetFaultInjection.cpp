/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Implementation of the fault-injection utilities for Inet.
 */

#include "InetFaultInjection.h"

#include <nlassert.h>

namespace chip {
namespace Inet {
namespace FaultInjection {

static nl::FaultInjection::Record sFaultRecordArray[kFault_NumItems];
static class nl::FaultInjection::Manager sInetFaultInMgr;
static const nl::FaultInjection::Name sManagerName  = "Inet";
static const nl::FaultInjection::Name sFaultNames[] = {
    "DNSResolverNew",
    "Send",
    "SendNonCritical",
};

/**
 * Get the singleton FaultInjection::Manager for Inet faults
 */
nl::FaultInjection::Manager & GetManager()
{
    if (0 == sInetFaultInMgr.GetNumFaults())
    {
        sInetFaultInMgr.Init(kFault_NumItems, sFaultRecordArray, sManagerName, sFaultNames);
    }
    return sInetFaultInMgr;
}

} // namespace FaultInjection
} // namespace Inet
} // namespace chip
