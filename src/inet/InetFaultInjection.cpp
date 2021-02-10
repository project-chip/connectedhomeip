/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      Implementation of the fault-injection utilities for Inet.
 */

#include "InetFaultInjection.h"

#include <nlassert.h>

#if INET_CONFIG_TEST

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

#endif // INET_CONFIG_TEST
