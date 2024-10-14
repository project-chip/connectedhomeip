/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/util/attribute-storage.h> // Needed for AttributeAccessInterfaceRegistry::Instance().Register

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Structs;
using namespace chip::app::Clusters::EnergyPreference::Attributes;

using Status = Protocols::InteractionModel::Status;

namespace {

class EnergyPrefAttrAccess : public AttributeAccessInterface
{
public:
    EnergyPrefAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), EnergyPreference::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

EnergyPrefAttrAccess gEnergyPrefAttrAccess;
Delegate * gsDelegate = nullptr;

CHIP_ERROR EnergyPrefAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == EnergyPreference::Id);
    EndpointId endpoint          = aPath.mEndpointId;
    uint32_t ourFeatureMap       = 0;
    const bool featureMapIsGood  = FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == Status::Success;
    const bool balanceSupported  = featureMapIsGood && ((ourFeatureMap & to_underlying(Feature::kEnergyBalance)) != 0);
    const bool lowPowerSupported = featureMapIsGood && ((ourFeatureMap & to_underlying(Feature::kLowPowerModeSensitivity)) != 0);

    switch (aPath.mAttributeId)
    {
    case EnergyBalances::Id:
        if (!balanceSupported)
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        if (gsDelegate != nullptr)
        {
            return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                do
                {
                    Percent step;
                    char buffer[64];
                    Optional<MutableCharSpan> label{ MutableCharSpan(buffer) };
                    if ((err = gsDelegate->GetEnergyBalanceAtIndex(endpoint, index, step, label)) == CHIP_NO_ERROR)
                    {
                        BalanceStruct::Type balance = { step, Optional<CharSpan>(label) };
                        ReturnErrorOnFailure(encoder.Encode(balance));
                        index++;
                    }
                } while (err == CHIP_NO_ERROR);

                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        return CHIP_ERROR_INCORRECT_STATE;
    case EnergyPriorities::Id:
        if (balanceSupported == false)
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

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
    case LowPowerModeSensitivities::Id:
        if (lowPowerSupported == false)
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        if (gsDelegate != nullptr)
        {
            return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                do
                {
                    Percent step;
                    char buffer[64];
                    Optional<MutableCharSpan> label{ MutableCharSpan(buffer) };
                    if ((err = gsDelegate->GetLowPowerModeSensitivityAtIndex(endpoint, index, step, label)) == CHIP_NO_ERROR)
                    {
                        BalanceStruct::Type balance = { step, Optional<CharSpan>(label) };
                        ReturnErrorOnFailure(encoder.Encode(balance));
                        index++;
                    }
                } while (err == CHIP_NO_ERROR);
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        return CHIP_ERROR_INCORRECT_STATE;
    default: // return CHIP_NO_ERROR and just read from the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

} // anonymous namespace

namespace chip::app::Clusters::EnergyPreference {

void SetDelegate(Delegate * aDelegate)
{
    gsDelegate = aDelegate;
}

Delegate * GetDelegate()
{
    return gsDelegate;
}

} // namespace chip::app::Clusters::EnergyPreference

Status MatterEnergyPreferenceClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                                      EmberAfAttributeType attributeType, uint16_t size,
                                                                      uint8_t * value)
{
    EndpointId endpoint = attributePath.mEndpointId;
    Delegate * delegate = GetDelegate();
    uint32_t ourFeatureMap;
    const bool featureMapIsGood  = FeatureMap::Get(attributePath.mEndpointId, &ourFeatureMap) == Status::Success;
    const bool balanceSupported  = featureMapIsGood && ((ourFeatureMap & to_underlying(Feature::kEnergyBalance)) != 0);
    const bool lowPowerSupported = featureMapIsGood && ((ourFeatureMap & to_underlying(Feature::kLowPowerModeSensitivity)) != 0);

    if (delegate == nullptr)
    {
        return Status::UnsupportedWrite;
    }

    switch (attributePath.mAttributeId)
    {
    case CurrentEnergyBalance::Id: {
        if (balanceSupported == false)
        {
            return Status::UnsupportedAttribute;
        }

        uint8_t index    = Encoding::Get8(value);
        size_t arraySize = delegate->GetNumEnergyBalances(endpoint);
        if (index >= arraySize)
        {
            return Status::ConstraintError;
        }

        return Status::Success;
    }

    case CurrentLowPowerModeSensitivity::Id: {
        if (lowPowerSupported == false)
        {
            return Status::UnsupportedAttribute;
        }

        uint8_t index    = Encoding::Get8(value);
        size_t arraySize = delegate->GetNumLowPowerModeSensitivities(endpoint);
        if (index >= arraySize)
        {
            return Status::ConstraintError;
        }

        return Status::Success;
    }
    default:
        return Status::Success;
    }
}

void MatterEnergyPreferencePluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gEnergyPrefAttrAccess);
}
