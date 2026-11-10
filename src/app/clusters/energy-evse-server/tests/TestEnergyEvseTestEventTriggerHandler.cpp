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

#include <gtest/gtest.h>

using namespace chip;
using namespace chip::app::Clusters::EnergyEvse;

namespace {

uint64_t EventTriggerForEndpoint(EndpointId endpoint, EnergyEvseTrigger trigger)
{
    return static_cast<uint64_t>(trigger) | (static_cast<uint64_t>(endpoint) << 32);
}

bool gInstanceHandlerCalled = false;

bool TestInstanceHandler(uint64_t eventTrigger)
{
    gInstanceHandlerCalled = (eventTrigger == static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn));
    return gInstanceHandlerCalled;
}

} // namespace

TEST(EnergyEvseTestEventTriggerHandler, InstanceModeFiltersByEndpoint)
{
    constexpr EndpointId kEndpoint = 7;
    EnergyEvseTestEventTriggerHandler handler(kEndpoint, TestInstanceHandler);

    gInstanceHandlerCalled = false;
    EXPECT_EQ(handler.HandleEventTrigger(EventTriggerForEndpoint(kEndpoint, EnergyEvseTrigger::kEVPluggedIn)), CHIP_NO_ERROR);
    EXPECT_TRUE(gInstanceHandlerCalled);

    gInstanceHandlerCalled = false;
    EXPECT_EQ(handler.HandleEventTrigger(EventTriggerForEndpoint(static_cast<EndpointId>(kEndpoint + 1),
                                                                 EnergyEvseTrigger::kEVPluggedIn)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(gInstanceHandlerCalled);

    gInstanceHandlerCalled = false;
    EXPECT_EQ(handler.HandleEventTrigger(static_cast<uint64_t>(EnergyEvseTrigger::kEVPluggedIn)), CHIP_NO_ERROR);
    EXPECT_TRUE(gInstanceHandlerCalled);
}

TEST(EnergyEvseTestEventTriggerHandler, GetEndpointIdFromEventTrigger)
{
    constexpr EndpointId kEndpoint = 42;
    const uint64_t encoded = EventTriggerForEndpoint(kEndpoint, EnergyEvseTrigger::kBasicFunctionality);
    EXPECT_EQ(GetEndpointIdFromEventTrigger(encoded), kEndpoint);
    EXPECT_EQ(GetEndpointIdFromEventTrigger(static_cast<uint64_t>(EnergyEvseTrigger::kBasicFunctionality)), static_cast<EndpointId>(0));
}
