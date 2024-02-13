/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <lib/core/CHIPError.h>
#include <stdint.h>

namespace chip {
namespace Tracing {

struct ScalarEvent
{
    static inline const uint32_t kEventDomainDiscovery = 1 << 16;
    static inline const uint32_t kEventDomainPASE = 2 << 16;
    static inline const uint32_t kEventDomainAttestation = 3 << 16;
    static inline const uint32_t kEventDomainCASE = 4 << 16;

    enum EventType {
        kDiscoveryOverBLE = (kEventDomainDiscovery | 1),
        kDiscoveryOnNetwork = (kEventDomainDiscovery | 2),
        kPASEConnectionEstablished = (kEventDomainPASE | 1),
        kPASEConnectionFailed = (kEventDomainPASE | 2),
        kAttestationResult = (kEventDomainAttestation | 1),
        kAttestationOverridden = (kEventDomainAttestation | 2),
        kCASEConnectionEstablished = (kEventDomainCASE | 1),
        kCASEConnectionFailed = (kEventDomainCASE | 2)
    };

    EventType eventType;
    uint32_t eventValue;

    ScalarEvent(EventType evtType, uint32_t evtValue = 0) 
        : eventType(evtType), eventValue(evtValue)
    {}

    ScalarEvent(EventType evtType, ChipError err) 
        : eventType(evtType), eventValue(err.AsInteger())
    {}

    static inline ScalarEvent make_event(EventType eventType, uint32_t eventValue = 0)
    {
        return ScalarEvent { eventType, eventValue };
    }
};

} // namespace Tracing
} // namespace chip
