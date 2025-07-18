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
namespace {
bool NullableCharSpanEqual(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b)
{
    if (a.IsNull() || b.IsNull())
    {
        return a.IsNull() == b.IsNull();
    }

    return a.Value().data_equal(b.Value());
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

Instance::~Instance()
{
    Shutdown();
}

CHIP_ERROR Instance::Init()
{
    mMeterType.SetNull();
    mPointOfDelivery.SetNull();
    mMeterSerialNumber.SetNull();
    mProtocolVersion.SetNull();
    mPowerThreshold.SetNull();
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

CHIP_ERROR Instance::SetMeterType(const DataModel::Nullable<MeterTypeEnum> & newValue)
{
    if (newValue == mMeterType)
    {
        return CHIP_NO_ERROR;
    }
    if (!newValue.IsNull() && MeterTypeEnum::kUnknownEnumValue == EnsureKnownEnumValue(newValue.Value()))
    {
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    }
    mMeterType = newValue;

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterType::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetPointOfDelivery(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanEqual(newValue, mPointOfDelivery))
    {
        return CHIP_NO_ERROR;
    }

    const size_t len = newValue.IsNull() ? 0 : newValue.Value().size();
    if (sizeof(mPointOfDeliveryBuf) < len)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!newValue.IsNull())
    {
        memmove(mPointOfDeliveryBuf, newValue.Value().data(), len);
        mPointOfDelivery = MakeNullable(CharSpan(mPointOfDeliveryBuf, len));
    }
    else
    {
        mPointOfDelivery.SetNull();
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PointOfDelivery::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanEqual(newValue, mMeterSerialNumber))
    {
        return CHIP_NO_ERROR;
    }

    const size_t len = newValue.IsNull() ? 0 : newValue.Value().size();
    if (sizeof(mPointOfDeliveryBuf) < len)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!newValue.IsNull())
    {
        memmove(mMeterSerialNumberBuf, newValue.Value().data(), len);
        mMeterSerialNumber = MakeNullable(CharSpan(mMeterSerialNumberBuf, len));
    }
    else
    {
        mMeterSerialNumber.SetNull();
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterSerialNumber::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetProtocolVersion(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanEqual(newValue, mProtocolVersion))
    {
        return CHIP_NO_ERROR;
    }

    const size_t len = newValue.IsNull() ? 0 : newValue.Value().size();
    if (sizeof(mPointOfDeliveryBuf) < len)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!newValue.IsNull())
    {
        memmove(mProtocolVersionBuf, newValue.Value().data(), len);
        mProtocolVersion = MakeNullable(CharSpan(mProtocolVersionBuf, len));
    }
    else
    {
        mProtocolVersion.SetNull();
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, ProtocolVersion::Id);
    return CHIP_NO_ERROR;
}

bool operator==(const Globals::Structs::PowerThresholdStruct::Type & a, const Globals::Structs::PowerThresholdStruct::Type & b)
{
    return a.powerThreshold == b.powerThreshold && a.apparentPowerThreshold == b.apparentPowerThreshold &&
        a.powerThresholdSource == b.powerThresholdSource;
}

CHIP_ERROR Instance::SetPowerThreshold(const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & newValue)
{
    if (newValue.IsNull())
    {
        if (mPowerThreshold.IsNull())
        {
            return CHIP_NO_ERROR;
        }

        mPowerThreshold.SetNull();
    }
    else
    {
        if (!mPowerThreshold.IsNull() && mPowerThreshold.Value() == newValue.Value())
        {
            return CHIP_NO_ERROR;
        }

        if (!newValue.Value().powerThresholdSource.IsNull() &&
            Globals::PowerThresholdSourceEnum::kUnknownEnumValue ==
                EnsureKnownEnumValue(newValue.Value().powerThresholdSource.Value()))
        {
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }

        mPowerThreshold.SetNonNull(newValue.Value());
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThreshold::Id);
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "Meter Indication read attr %" PRIu32 " on endpoint %" PRIu32, static_cast<uint32_t>(aPath.mAttributeId),
                    static_cast<uint32_t>(mEndpointId));

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case MeterType::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeterType()));
        break;

    case PointOfDelivery::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetPointOfDelivery()));
        break;

    case MeterSerialNumber::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeterSerialNumber()));
        break;

    case ProtocolVersion::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetProtocolVersion()));
        break;

    case PowerThreshold::Id:
        if (HasFeature(Feature::kPowerThreshold))
            ReturnErrorOnFailure(aEncoder.Encode(GetPowerThreshold()));
        else
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterMeterIdentificationPluginServerInitCallback() {}
