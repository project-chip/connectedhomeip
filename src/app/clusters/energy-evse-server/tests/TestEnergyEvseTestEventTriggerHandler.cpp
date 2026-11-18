/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>

#include <lib/support/tests/ExtraPwTestMacros.h>

#include <optional>

using namespace chip;

namespace {

std::optional<uint64_t> gLastEventTrigger;

uint64_t EventTriggerForEndpoint(EndpointId endpoint, EnergyEvseTrigger trigger)
{
    return static_cast<uint64_t>(trigger) | (static_cast<uint64_t>(endpoint) << 32);
}

class TestEvse : public TestEventTriggerHandler
{
public:
    explicit TestEvse(EndpointId endpoint) : mEndpoint(endpoint) {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        const EndpointId endpoint = GetEndpointIdFromEventTrigger(eventTrigger);
        VerifyOrReturnError(endpoint == 0 || endpoint == mEndpoint, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(clearEndpointInEventTrigger(eventTrigger) == static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn),
                            CHIP_ERROR_INVALID_ARGUMENT);
        mPluggedIn = true;
        return CHIP_NO_ERROR;
    }

    bool IsPluggedIn() const { return mPluggedIn; }

private:
    const EndpointId mEndpoint;
    bool mPluggedIn = false;
};

} // namespace

bool HandleEnergyEvseTestEventTrigger(uint64_t eventTrigger)
{
    gLastEventTrigger = eventTrigger;
    return eventTrigger == static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn);
}

TEST(EnergyEvseTestEventTriggerHandler, RoutesToMatchingInstance)
{
    SimpleTestEventTriggerDelegate delegate;
    TestEvse first(7);
    TestEvse second(8);
    ASSERT_SUCCESS(delegate.AddHandler(&first));
    ASSERT_SUCCESS(delegate.AddHandler(&second));

    EXPECT_EQ(delegate.HandleEventTriggers(EventTriggerForEndpoint(9, EnergyEvseTrigger::kEVPluggedIn)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(first.IsPluggedIn());
    EXPECT_FALSE(second.IsPluggedIn());

    EXPECT_EQ(delegate.HandleEventTriggers(EventTriggerForEndpoint(8, EnergyEvseTrigger::kBasicFunctionality)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(first.IsPluggedIn());
    EXPECT_FALSE(second.IsPluggedIn());

    EXPECT_SUCCESS(delegate.HandleEventTriggers(EventTriggerForEndpoint(8, EnergyEvseTrigger::kEVPluggedIn)));
    EXPECT_FALSE(first.IsPluggedIn());
    EXPECT_TRUE(second.IsPluggedIn());

    EXPECT_SUCCESS(delegate.HandleEventTriggers(EventTriggerForEndpoint(7, EnergyEvseTrigger::kEVPluggedIn)));
    EXPECT_TRUE(first.IsPluggedIn());
    EXPECT_TRUE(second.IsPluggedIn());
}

TEST(EnergyEvseTestEventTriggerHandler, UnscopedTriggerUsesFirstAcceptingInstance)
{
    SimpleTestEventTriggerDelegate delegate;
    TestEvse first(7);
    TestEvse second(8);
    ASSERT_SUCCESS(delegate.AddHandler(&first));
    ASSERT_SUCCESS(delegate.AddHandler(&second));

    EXPECT_SUCCESS(delegate.HandleEventTriggers(static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn)));
    EXPECT_TRUE(first.IsPluggedIn());
    EXPECT_FALSE(second.IsPluggedIn());
}

TEST(EnergyEvseTestEventTriggerHandler, RejectsUnsupportedUnscopedTrigger)
{
    SimpleTestEventTriggerDelegate delegate;
    TestEvse first(7);
    TestEvse second(8);
    ASSERT_SUCCESS(delegate.AddHandler(&first));
    ASSERT_SUCCESS(delegate.AddHandler(&second));

    EXPECT_EQ(delegate.HandleEventTriggers(static_cast<uint64_t>(EnergyEvseTrigger::kBasicFunctionality)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(first.IsPluggedIn());
    EXPECT_FALSE(second.IsPluggedIn());
}

TEST(EnergyEvseTestEventTriggerHandler, GlobalHandlerClearsEndpoint)
{
    EnergyEvseTestEventTriggerHandler handler;
    constexpr uint64_t kPluggedIn = static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn);

    gLastEventTrigger.reset();
    EXPECT_SUCCESS(handler.HandleEventTrigger(kPluggedIn));
    EXPECT_EQ(gLastEventTrigger, std::make_optional(kPluggedIn));

    gLastEventTrigger.reset();
    EXPECT_SUCCESS(handler.HandleEventTrigger(EventTriggerForEndpoint(7, EnergyEvseTrigger::kEVPluggedIn)));
    EXPECT_EQ(gLastEventTrigger, std::make_optional(kPluggedIn));

    gLastEventTrigger.reset();
    EXPECT_EQ(handler.HandleEventTrigger(0xABCD123489ABCDEF), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(gLastEventTrigger, std::make_optional<uint64_t>(0xABCD000089ABCDEF));
}
