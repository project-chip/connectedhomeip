/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
#include <device/types/dishwasher/Dishwasher.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class EmulatedDishwasher : public Dishwasher
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit EmulatedDishwasher(const Context & context) :
        Dishwasher(Dishwasher::Config{
            .operationalStateDelegate = mOpStateDelegate,
            .modeDelegate             = mModeDelegate,
            .diagnosticDataProvider   = context.diagnosticDataProvider,
        }),
        mOpStateDelegate(context.timerDelegate)
    {}

    ~EmulatedDishwasher() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override
    {
        ReturnErrorOnFailure(Dishwasher::Register(endpoint, provider, composition));
        mOpStateDelegate.SetCluster(&OperationalState());
        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        mOpStateDelegate.SetCluster(nullptr);
        Dishwasher::Unregister(provider);
    }

private:
    Clusters::OperationalState::EmulatedOperationalStateDelegate mOpStateDelegate;
    DishwasherModeDelegate mModeDelegate;
};

} // namespace chip::app
