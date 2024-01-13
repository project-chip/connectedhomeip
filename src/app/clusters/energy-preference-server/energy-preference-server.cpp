/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "energy-preference-server.h"

#include <app/util/af.h>

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Structs;
using namespace chip::app::Clusters::EnergyPreference::Attributes;

namespace {

class EnergyPrefAttrAccess : public AttributeAccessInterface
{
public:
    EnergyPrefAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), EnergyPreference::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};

EnergyPrefAttrAccess gEnergyPrefAttrAccess;
EnergyPreferenceDelegate *gsDelegate;

CHIP_ERROR EnergyPrefAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == EnergyPreference::Id);
    EndpointId endpoint = aPath.mEndpointId;

    switch (aPath.mAttributeId)
    {
        case EnergyBalances::Id: {
            if (gsDelegate != nullptr)
            {
                return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
                    BalanceStruct::Type balance;
                    size_t index   = 0;
                    CHIP_ERROR err = CHIP_NO_ERROR;
                    while ((err = gsDelegate->GetEnergyBalanceAtIndex(endpoint, index, balance)) == CHIP_NO_ERROR)
                    {
                        ReturnErrorOnFailure(encoder.Encode(balance));
                        index++;
                    }
                    if (err == CHIP_ERROR_NOT_FOUND)
                    {
                        return CHIP_NO_ERROR;
                    }
                    return err;
                });
            }
            return CHIP_ERROR_INCORRECT_STATE;
        }
        break;
        case EnergyPriorities::Id: {
            if (gsDelegate != nullptr)
            {
                return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
                    EnergyPriorityEnum priority;
                    size_t index   = 0;
                    CHIP_ERROR err = CHIP_NO_ERROR;
                    while ((err = gsDelegate->GetEnergyPriorityAtIndex(endpoint, index, priority)) == CHIP_NO_ERROR)
                    {
                        ReturnErrorOnFailure(encoder.Encode(priority));
                        index++;
                    }
                    if (err == CHIP_ERROR_NOT_FOUND)
                    {
                        return CHIP_NO_ERROR;
                    }
                    return err;
                });
            }
            return CHIP_ERROR_INCORRECT_STATE;
        }
        break;
        case LowPowerModeSensitivities::Id: {
            if (gsDelegate != nullptr)
            {
                return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
                    BalanceStruct::Type balance;
                    size_t index   = 0;
                    CHIP_ERROR err = CHIP_NO_ERROR;
                    while ((err = gsDelegate->GetLowPowerModeSensitivityAtIndex(endpoint, index, balance)) == CHIP_NO_ERROR)
                    {
                        ReturnErrorOnFailure(encoder.Encode(balance));
                        index++;
                    }
                    if (err == CHIP_ERROR_NOT_FOUND)
                    {
                        return CHIP_NO_ERROR;
                    }
                    return err;
                });
            }
            return CHIP_ERROR_INCORRECT_STATE;
        }
        break;
        default: // return CHIP_NO_ERROR and just read from the attribute store in default
            break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyPrefAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    // return CHIP_NO_ERROR and just write to the attribute store in default
    return CHIP_NO_ERROR;
}

} // anonymous namespace

void SetMatterEnergyPreferencesDelegate(EnergyPreferenceDelegate * aDelegate)
{
    gsDelegate = aDelegate;
}

EnergyPreferenceDelegate * GetMatterEnergyPreferencesDelegate()
{
    return gsDelegate;
}

void MatterEnergyPreferencePluginServerInitCallback()
{
    registerAttributeAccessOverride(&gEnergyPrefAttrAccess);
    // Get the delegate?
}
