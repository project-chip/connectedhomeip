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
bool NullableCharSpanCompare(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b)
{
    if (a.IsNull() && b.IsNull())
    {
        return true;
    }

    if (!a.IsNull() && !b.IsNull())
    {
        return a.Value().data_equal(b.Value());
    }

    return false;
}

CHIP_ERROR NullableCharSpanCopy(DataModel::Nullable<CharSpan> & dst, const DataModel::Nullable<CharSpan> & src)
{
    const size_t len = src.IsNull() ? 0 : src.Value().size();
    if (64 < len)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!dst.IsNull())
    {
        chip::Platform::MemoryFree(const_cast<char *>(dst.Value().data()));
        dst.SetNull();
    }

    if (!src.IsNull())
    {
        if (auto * str = static_cast<char *>(chip::Platform::MemoryAlloc(1 + len)))
        {
            memcpy(str, src.Value().data(), len);
            str[len] = 0;
            dst = MakeNullable(CharSpan(str, len));
        }
        else
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    return CHIP_NO_ERROR;
}
    } // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

Instance::~Instance()
{
    Shutdown();
    if (!mPointOfDelivery.IsNull())
    {
        chip::Platform::MemoryFree(const_cast<char *>(mPointOfDelivery.Value().data()));
    }
    if (!mMeterSerialNumber.IsNull())
    {
        chip::Platform::MemoryFree(const_cast<char *>(mMeterSerialNumber.Value().data()));
    }
    if (!mProtocolVersion.IsNull())
    {
        chip::Platform::MemoryFree(const_cast<char *>(mProtocolVersion.Value().data()));
    }
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mEndpointId, Id));
    SetMeterType(std::nullopt);
    SetPointOfDelivery(std::nullopt);
    SetMeterSerialNumber(std::nullopt);
    SetProtocolVersion(std::nullopt);
    SetPowerThreshold(std::nullopt);
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
    if (newValue.IsNull())
    {
        if (mMeterType.IsNull())
        {
            return CHIP_NO_ERROR;
        }

        mMeterType.SetNull();
    }
    else
    {
        if (!mMeterType.IsNull() && mMeterType.Value() == newValue.Value())
        {
            return CHIP_NO_ERROR;
        }

        if(MeterTypeEnum::kUnknownEnumValue == EnsureKnownEnumValue(newValue.Value()))
        {
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }

        mMeterType.SetNonNull(newValue.Value());
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterType::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetPointOfDelivery(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mPointOfDelivery))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(mPointOfDelivery, newValue);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PointOfDelivery::Id);
    }
    return ret;
}

CHIP_ERROR Instance::SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mMeterSerialNumber))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(mMeterSerialNumber, newValue);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterSerialNumber::Id);
    }
    return ret;
}

CHIP_ERROR Instance::SetProtocolVersion(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mProtocolVersion))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(mProtocolVersion, newValue);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, ProtocolVersion::Id);
    }
    return ret;
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
        if (!mPowerThreshold.IsNull() && (newValue.Value().powerThreshold == mPowerThreshold.Value().powerThreshold &&
            newValue.Value().apparentPowerThreshold == mPowerThreshold.Value().apparentPowerThreshold &&
            newValue.Value().powerThresholdSource == mPowerThreshold.Value().powerThresholdSource))
        {
            return CHIP_NO_ERROR;
        }

        if (!(newValue.ExistingValueInEncodableRange() && (newValue.Value().powerThresholdSource.IsNull() ||
            Globals::PowerThresholdSourceEnum::kUnknownEnumValue != EnsureKnownEnumValue(newValue.Value().powerThresholdSource.Value()))))
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        mPowerThreshold.SetNonNull(newValue.Value());
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThreshold::Id);
    return CHIP_NO_ERROR;
}

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
        ReturnErrorOnFailure(aEncoder.Encode(GetMeterType()));
        break;

    case PointOfDelivery::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetPointOfDelivery()));
        break;

    case MeterSerialNumber::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeterSerialNumber()));;
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