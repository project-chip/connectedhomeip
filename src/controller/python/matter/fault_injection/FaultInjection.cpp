/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/CHIPFaultInjection.h>

extern "C" int32_t pychip_faultinjection_fail_at_fault(uint32_t faultID, uint32_t numCallsToSkip, uint32_t numCallsToFail,
                                                       bool takeMutex)
{
    // Only ChipFaults (defined in src/lib/support/CHIPFaultInjection.h) are implemented, Implement others by adding calls to thier
    // fault injection managers (SystemFauls and InetFaults)
    return chip::FaultInjection::GetManager().FailAtFault(faultID, numCallsToSkip, numCallsToFail, takeMutex);
}

extern "C" uint32_t pychip_faultinjection_get_fault_counter(uint32_t faultID)
{

    return chip::FaultInjection::GetFaultCounter(faultID);
}

extern "C" void pychip_faultinjection_reset_fault_counters(void)
{
    return chip::FaultInjection::GetManager().ResetFaultCounters();
}

extern "C" int pychip_faultinjection_get_num_faults()
{
    return chip::FaultInjection::kNumChipFaultsFromEnum;
}
