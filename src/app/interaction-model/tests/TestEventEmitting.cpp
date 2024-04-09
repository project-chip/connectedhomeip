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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/interaction-model/events.h>

#include <gtest/gtest.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::InteractionModel;

constexpr uint32_t kFakeSoftwareVersion = 0x1234abcd;

class LogOnlyEvents : public Events
{
public:
    virtual CHIP_ERROR EmitEvent(EventLoggingDelegate * eventContentWriter, const EventOptions & options,
                                 EventNumber & generatedEventNumber)
    {

        generatedEventNumber = ++mCurrentEventNumber;

        return CHIP_NO_ERROR;
    }

    EventNumber CurrentEventNumber() const { return mCurrentEventNumber; }

private:
    EventNumber mCurrentEventNumber = 0;
};

} // namespace

TEST(TestInteractionModelEventEmitting, TodoTest)
{
    LogOnlyEvents logOnlyEvents;
    Events * events = &logOnlyEvents;

    chip::app::Clusters::BasicInformation::Events::StartUp::Type event{ kFakeSoftwareVersion };

    EventNumber n1 = events->EmitEvent(event, 0);
    ASSERT_EQ(n1, logOnlyEvents.CurrentEventNumber());


    EventNumber n2 = events->EmitEvent(event, 1);
    ASSERT_EQ(n2, logOnlyEvents.CurrentEventNumber());
    ASSERT_NE(n1, logOnlyEvents.CurrentEventNumber());
}
