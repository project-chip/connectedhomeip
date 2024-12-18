/*
 *
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

#include <app/ConcreteEventPath.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {

/**
 *   Interface that EventManagement can use to notify when events are generated and may need reporting.
 *
 */
class EventReporter
{
public:
    virtual ~EventReporter() = default;

    /**
     *  Notify that an event was generated.
     *
     * @param[in] aPath           The path that identifies the kind of event that was generated.
     * @param[in] aBytesConsumed  The number of bytes needed to store the event in EventManagement.
     */
    CHIP_ERROR virtual NewEventGenerated(ConcreteEventPath & aPath, uint32_t  aBytesConsumed) = 0;
};

} // namespace app
} // namespace chip
