/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *   Configuration of the CHIP Event Logging.
 *
 */

#pragma once

#include "EventLoggingTypes.h"

namespace chip {
namespace app {
namespace reporting {

/**
 * @brief
 *   LoggingConfiguration encapsulates the configurable component
 *   of the chip Event Logging subsystem.
 */
class LoggingConfiguration
{
public:
    LoggingConfiguration(void);

    static LoggingConfiguration & GetInstance(void);

    // only log with higher priority than mGlobalPriority can be logged
    PriorityLevel GetGlobalPriorityLevel() const { return mGlobalPriority; };
    void SetGlobalPriorityLevel(PriorityLevel aPriorityLevel) { mGlobalPriority = aPriorityLevel; };

private:
    PriorityLevel mGlobalPriority;
};

} // namespace reporting
} // namespace app
} // namespace chip
