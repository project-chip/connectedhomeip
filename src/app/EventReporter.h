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

/**
 * @file
 *
 * @brief
 *   Define EventReporter interface. Event reproter is used by EventManagement to notify that events are ready to be reported.
 *   Usually this is implemented by the Reporting Engine to find the proper ReadHandlers and deliver the events.
 *
 */
#pragma once

#include <app/ConcreteEventPath.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {

class EventReporter
{
public:
    virtual ~EventReporter() = default;

    /**
     * @brief
     *  Notify of a new event generated.
     *
     * @param[in] aPath          The path to the event.
     * @param[in] aBytesWritten  Bytes that the event is written into the buffer in EventManagement.
     */
    CHIP_ERROR virtual NewEventGenerated(ConcreteEventPath & aPath, uint32_t aBytesWritten) = 0;
};

} // namespace app
} // namespace chip
