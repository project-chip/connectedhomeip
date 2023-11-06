/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <map>

#include <app/tests/suites/pics/PICSBooleanExpressionParser.h>
#include <app/tests/suites/pics/PICSBooleanReader.h>
#include <lib/core/Optional.h>
#include <lib/support/logging/CHIPLogging.h>

class PICSChecker
{
public:
    bool ShouldSkip(const char * expression)
    {
        // If there is no PICS configuration file, considers that nothing should be skipped.
        if (!PICS.HasValue())
        {
            return false;
        }

        std::map<std::string, bool> pics(PICS.Value());
        bool shouldSkip = !PICSBooleanExpressionParser::Eval(expression, pics);
        if (shouldSkip)
        {
            ChipLogProgress(chipTool, " **** Skipping: %s == false\n", StringOrNullMarker(expression));
        }
        return shouldSkip;
    }

    chip::Optional<std::map<std::string, bool>> PICS;
};
