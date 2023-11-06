/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <string>

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/UnitTestUtils.h>

class TestRunner
{
public:
    TestRunner(const char * name, uint16_t testCount) : mTestName(name), mTestCount(testCount), mTestIndex(0) {}
    virtual ~TestRunner(){};

    void LogStart() { ChipLogProgress(chipTool, " ***** Test Start : %s\n", StringOrNullMarker(mTestName)); }

    void LogStep(uint32_t stepNumber, const char * stepName)
    {
        ChipLogProgress(chipTool, " ***** Test Step %u : %s\n", stepNumber, StringOrNullMarker(stepName));
    }

    void LogEnd(std::string message, CHIP_ERROR err)
    {
        if (CHIP_NO_ERROR == err)
        {
            ChipLogProgress(chipTool, " **** Test Complete: %s\n", message.c_str());
        }
        else
        {
            ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
        }
    }

    virtual CHIP_ERROR DoTestStep(uint16_t testIndex)                            = 0;
    virtual void Exit(std::string message, CHIP_ERROR err = CHIP_ERROR_INTERNAL) = 0;

    void NextTest()
    {
        if (mTestSubStepIndex != mTestSubStepCount)
        {
            Exit(mTestName, CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        mTestSubStepIndex = 0;
        mTestSubStepCount = 0;

        if (0 == mTestIndex)
        {
            LogStart();
        }

        if (mTestCount == mTestIndex)
        {
            Exit(mTestName, CHIP_NO_ERROR);
            return;
        }

        if (mDelayInMs.HasValue())
        {
            chip::test_utils::SleepMillis(mDelayInMs.Value());
        }

        // Ensure we increment mTestIndex before we start running the relevant
        // command.  That way if we lose the timeslice after we send the message
        // but before our function call returns, we won't end up with an
        // incorrect mTestIndex value observed when we get the response.
        auto err = DoTestStep(mTestIndex++);
        if (CHIP_NO_ERROR != err)
        {
            Exit(chip::ErrorStr(err));
        }
    }

protected:
    const char * mTestName;
    const uint16_t mTestCount;
    std::atomic_uint16_t mTestIndex;
    chip::Optional<uint64_t> mDelayInMs;

    uint16_t mTestSubStepIndex = 0;
    uint16_t mTestSubStepCount = 0;
};
