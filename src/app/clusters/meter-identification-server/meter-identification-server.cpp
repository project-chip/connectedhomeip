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
#include "meter-identification-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;
using namespace chip::app::Clusters::MeterIdentification::Structs;

using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::MeterIdentification {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    unregisterAttributeAccessOverride(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

static CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case MeterType::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetMeterType()));
        break;

    case CustomerName::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetCustomerName()));
        break;

    case UtilityName::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetUtilityName()));
        break;

    case PointOfDelivery::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPointOfDelivery()));
        break;

    case PowerThreshold::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPowerThreshold()));
        break;

    case PowerThresholdSource::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPowerThresholdSource()));
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CharSpan str;

    switch (aPath.mAttributeId)
    {
    case CustomerName::Id:
        ReturnErrorOnFailure(aDecoder.Decode(str));
        status = mDelegate.SetCustomerName(str);
        break;

    default:
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::MeterIdentification
