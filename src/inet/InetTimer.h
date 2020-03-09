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
 *      This file defines macros, constants, data structures, and
 *      functions that represent a wrapper around one-shot timer objects
 *
 */

#ifndef INETTIMER_H
#define INETTIMER_H

#include <InetLayer/InetConfig.h>

#if !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#error "#include <InetLayer/InetTimer.h> // while !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES."

#else // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#include <SystemLayer/SystemTimer.h>

/**
 *  Number of nanoseconds in a microsecond
 */

#define INET_NANOSECONDS_PER_MICROSECOND    nl::Weave::System::kTimerFactor_nano_per_micro

/**
 *  Number of nanoseconds in a millisecond
 */
#define INET_NANOSECONDS_PER_MILLISECOND    nl::Weave::System::kTimerFactor_nano_per_milli

/**
 *  Number of microseconds in a millisecond
 */
#define INET_MICROSECONDS_PER_MILLISECOND   nl::Weave::System::kTimerFactor_micro_per_milli

/**
 *  Number of milliseconds in a second
 */
#define INET_MILLISECONDS_PER_SECOND        nl::Weave::System::kTimerFactor_milli_per_unit

/**
 *  Number of microseconds in a second
 */
#define INET_MICROSECONDS_PER_SECOND        nl::Weave::System::kTimerFactor_micro_per_unit

namespace nl {
namespace Inet {

class InetLayer;

class InetTimer
{
    friend class InetLayer;

    InetTimer(void);
    InetTimer(const InetTimer&);
    InetTimer& operator =(const InetTimer&);

public:
    typedef Weave::System::Timer::Epoch Time;

    static Time GetTimeMillis(void);

protected:
};

inline InetTimer::Time InetTimer::GetTimeMillis(void)
{
    return Weave::System::Timer::GetCurrentEpoch();
}

} // namespace Inet
} // namespace nl

#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#endif // !defined(INETTIMER_H)
