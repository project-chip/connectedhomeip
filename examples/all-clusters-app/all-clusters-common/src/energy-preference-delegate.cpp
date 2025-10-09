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

#include <app/clusters/energy-preference-server/energy-preference-server.h>

using namespace chip;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Structs;

static BalanceStruct::Type gsEnergyBalances[] = {
    { .step = 0, .label = Optional<chip::CharSpan>("Efficient"_span) },
    { .step = 50, .label = Optional<chip::CharSpan>() },
    { .step = 100, .label = Optional<chip::CharSpan>("Comfort"_span) },
};

static BalanceStruct::Type gsPowerBalances[] = {
    { .step = 0, .label = Optional<chip::CharSpan>("1 Minute"_span) },
    { .step = 12, .label = Optional<chip::CharSpan>("5 Minutes"_span) },
    { .step = 24, .label = Optional<chip::CharSpan>("10 Minutes"_span) },
    { .step = 36, .label = Optional<chip::CharSpan>("15 Minutes"_span) },
    { .step = 48, .label = Optional<chip::CharSpan>("20 Minutes"_span) },
    { .step = 60, .label = Optional<chip::CharSpan>("25 Minutes"_span) },
    { .step = 70, .label = Optional<chip::CharSpan>("30 Minutes"_span) },
    { .step = 80, .label = Optional<chip::CharSpan>("60 Minutes"_span) },
    { .step = 90, .label = Optional<chip::CharSpan>("120 Minutes"_span) },
    { .step = 100, .label = Optional<chip::CharSpan>("Never"_span) },
};

// assumes it'll be the only delegate for it's lifetime.
struct EPrefDelegate : public Delegate
{
    EPrefDelegate();
    virtual ~EPrefDelegate();

    CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                       chip::Optional<chip::MutableCharSpan> & aOutLabel) override;
    CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, EnergyPriorityEnum & priority) override;
    CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                                 chip::Optional<chip::MutableCharSpan> & aOutLabel) override;

    size_t GetNumEnergyBalances(chip::EndpointId aEndpoint) override;
    size_t GetNumLowPowerModeSensitivities(chip::EndpointId aEndpoint) override;
};

EPrefDelegate::EPrefDelegate() : Delegate()
{
    VerifyOrDie(GetDelegate() == nullptr);
    SetDelegate(this);
}

EPrefDelegate::~EPrefDelegate()
{
    VerifyOrDie(GetDelegate() == this);
    SetDelegate(nullptr);
}

size_t EPrefDelegate::GetNumEnergyBalances(chip::EndpointId aEndpoint)
{
    return (MATTER_ARRAY_SIZE(gsEnergyBalances));
}

size_t EPrefDelegate::GetNumLowPowerModeSensitivities(chip::EndpointId aEndpoint)
{
    return (MATTER_ARRAY_SIZE(gsEnergyBalances));
}

CHIP_ERROR
EPrefDelegate::GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                       chip::Optional<chip::MutableCharSpan> & aOutLabel)
{
    if (aIndex < GetNumEnergyBalances(aEndpoint))
    {
        aOutStep = gsEnergyBalances[aIndex].step;
        if (gsEnergyBalances[aIndex].label.HasValue())
        {
            chip::CopyCharSpanToMutableCharSpan(gsEnergyBalances[aIndex].label.Value(), aOutLabel.Value());
        }
        else
        {
            aOutLabel.ClearValue();
        }
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
EPrefDelegate::GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, EnergyPriorityEnum & priority)
{
    static EnergyPriorityEnum priorities[] = { EnergyPriorityEnum::kEfficiency, EnergyPriorityEnum::kComfort };

    if (aIndex < MATTER_ARRAY_SIZE(priorities))
    {
        priority = priorities[aIndex];
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
EPrefDelegate::GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                                 chip::Optional<chip::MutableCharSpan> & aOutLabel)
{
    if (aIndex < GetNumLowPowerModeSensitivities(aEndpoint))
    {
        aOutStep = gsPowerBalances[aIndex].step;
        if (gsPowerBalances[aIndex].label.HasValue())
        {
            chip::CopyCharSpanToMutableCharSpan(gsPowerBalances[aIndex].label.Value(), aOutLabel.Value());
        }
        else
        {
            aOutLabel.ClearValue();
        }
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

static EPrefDelegate gsDelegate;
