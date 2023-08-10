/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/config.h>
#include <laundry-washer-controls-delegate-impl.h>

using namespace chip;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::Protocols::InteractionModel;

const CharSpan LaundryWasherControlDelegate::spinSpeedsNameOptions[] = {
    CharSpan::fromCharString("Off"),
    CharSpan::fromCharString("Low"),
    CharSpan::fromCharString("Medium"),
    CharSpan::fromCharString("High"),
};

const NumberOfRinsesEnum LaundryWasherControlDelegate::supportRinsesOptions[] = {
    NumberOfRinsesEnum::kNormal,
    NumberOfRinsesEnum::kExtra,
};

LaundryWasherControlDelegate LaundryWasherControlDelegate::instance;

CHIP_ERROR LaundryWasherControlDelegate::GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed)
{
    if (index >= ArraySize(spinSpeedsNameOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(LaundryWasherControlDelegate::spinSpeedsNameOptions[index], spinSpeed);
}

CHIP_ERROR LaundryWasherControlDelegate::GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse)
{
    if (index >= ArraySize(supportRinsesOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    supportedRinse = LaundryWasherControlDelegate::supportRinsesOptions[index];
    return CHIP_NO_ERROR;
}

Status LaundryWasherControlDelegate::PreAttributeCheck(AttributeId attributeId, uint16_t size, uint8_t * value)
{
    switch (attributeId)
    {
    case Attributes::SpinSpeedCurrent::Id: {
        if (*value >= ArraySize(spinSpeedsNameOptions))
        {
            return Status::ConstraintError;
        }
        return Status::Success;
    }
    case Attributes::NumberOfRinses::Id: {
        if (size != sizeof(NumberOfRinsesEnum))
        {
            return Status::InvalidDataType;
        }
        for (uint16_t index = 0; index < ArraySize(supportRinsesOptions); index++)
        {
            if (static_cast<NumberOfRinsesEnum>(*value) == LaundryWasherControlDelegate::supportRinsesOptions[index])
            {
                return Status::Success;
            }
        }
        return Status::InvalidInState;
    }
    case Attributes::SpinSpeeds::Id:
    case Attributes::SupportedRinses::Id:
        return Status::UnsupportedWrite;
    default:
        return Status::UnsupportedAttribute;
    }
}

void LaundryWasherControlDelegate::Init(EndpointId endpointId)
{
    NumberOfRinsesEnum supportedRinse;
    GetSupportedRinseAtIndex(kDefaultRinseIndex, supportedRinse);
    LaundryWasherControlsServer::Instance().SetNumberOfRinses(endpointId, supportedRinse);
    LaundryWasherControlsServer::Instance().SetSpinSpeedCurrent(endpointId, DataModel::Nullable<uint8_t>(kDefaultSpinSpeedIndex));
}
