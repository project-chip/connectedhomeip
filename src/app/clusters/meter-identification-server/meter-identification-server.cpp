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
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(const Feature & aFeature) const
{
    return mFeature.Has(aFeature);
}

// static CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
//{
//     ReturnErrorOnFailure(status);
//     return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
// }

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "Meter Indication read attr %d", aPath.mAttributeId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case MeterType::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetMeterType()));
        break;

    case PointOfDelivery::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPointOfDelivery()));
        break;

    case MeterSerialNumber::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetMeterSerialNumber()));
        break;

    case ProtocolVersion::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetProtocolVersion()));
        break;

    case PowerThreshold::Id:
        if (HasFeature(Feature::kPowerThreshold))
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPowerThreshold()));
        else
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    /*

        switch (aPath.mAttributeId)
        {

        default:
            break;
        }
    */
    return CHIP_NO_ERROR;
}

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterMeterIdentificationPluginServerInitCallback() {}
