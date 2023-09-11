/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
