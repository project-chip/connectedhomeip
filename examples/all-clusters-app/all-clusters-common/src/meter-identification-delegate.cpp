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

#include "meter-identification-delegate.h"
#include <app/reporting/reporting.h>

#include <app/clusters/meter-identification-server/meter-identification-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;

using chip::app::Clusters::MeterIdentification::MeterTypeEnum;
using Feature = chip::app::Clusters::MeterIdentification::Feature;

namespace chip::app::Clusters::MeterIdentification {
    void RestartServer(uint32_t features);
}

CHIP_ERROR MeterIdentificationInstance::Init()
{
    return Instance::Init();
}

void MeterIdentificationInstance::Shutdown()
{
    Instance::Shutdown();
}

// --------------- Internal Attribute Set APIs

chip::app::Clusters::MeterIdentification::MeterIdentificationDelegate::~MeterIdentificationDelegate()
{
    if (!mPointOfDelivery.IsNull())
    {
        chip::Platform::MemoryFree((void *) mPointOfDelivery.Value().data());
        mPointOfDelivery.SetNull();
    }

    if (!mMeterSerialNumber.IsNull())
    {
        chip::Platform::MemoryFree((void *) mMeterSerialNumber.Value().data());
        mMeterSerialNumber.SetNull();
    }

    if (!mProtocolVersion.IsNull())
    {
        chip::Platform::MemoryFree((void *) mProtocolVersion.Value().data());
        mProtocolVersion.SetNull();
    }
}

void MeterIdentificationDelegate::Init()
{
    SetMeterType(std::nullopt);
    SetPointOfDelivery(std::nullopt);
    SetMeterSerialNumber(std::nullopt);
    SetProtocolVersion(std::nullopt);
    SetPowerThreshold(std::nullopt);
}

CHIP_ERROR MeterIdentificationDelegate::LoadJson(const Json::Value & root)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;
    Json::Value value;

    if (root.isMember("Features"))
    {
        value = root.get("Features", Json::Value());
        if (value.isUInt())
        {
            if(1 >= value.asUInt())
            {
                RestartServer(value.asUInt());
                return ret;
            }
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("MeterType"))
    {
        value = root.get("MeterType", Json::Value());
        if(value.isUInt())
        {
            ret = CHIP_NO_ERROR == ret ? SetMeterType(MakeNullable(static_cast<MeterTypeEnum>(value.asUInt()))) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("PointOfDelivery"))
    {
        value = root.get("PointOfDelivery", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? SetPointOfDelivery(CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("MeterSerialNumber"))
    {
        value = root.get("MeterSerialNumber", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? SetMeterSerialNumber(CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("ProtocolVersion"))
    {
        value = root.get("ProtocolVersion", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? SetProtocolVersion(CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("PowerThreshold"))
    {
        value = root.get("PowerThreshold", Json::Value());
        if (!value.empty() && value.isObject())
        {
            Structs::PowerThresholdStruct::Type powerThreshopld;
            const CHIP_ERROR error = JsonToPowerThreshold(value, powerThreshopld);
            if (CHIP_NO_ERROR == error)
            {
                ret = CHIP_NO_ERROR == ret ? SetPowerThreshold(MakeNullable(powerThreshopld)) : ret;
            }
            else
            {
                ret = error;
            }
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    return ret;
}

CHIP_ERROR MeterIdentificationDelegate::SetMeterType(const DataModel::Nullable<MeterTypeEnum> & newValue)
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

        if(static_cast<std::underlying_type<MeterTypeEnum>::type>(MeterTypeEnum::kUnknownEnumValue) <=
            static_cast<std::underlying_type<MeterTypeEnum>::type>(newValue.Value()))
        {
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }

        mMeterType.SetNonNull(newValue.Value());
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterType::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPointOfDelivery(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mPointOfDelivery))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(newValue, mPointOfDelivery);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PointOfDelivery::Id);
    }
    return ret;
}

CHIP_ERROR MeterIdentificationDelegate::SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mMeterSerialNumber))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(newValue, mMeterSerialNumber);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterSerialNumber::Id);
    }
    return ret;
}

CHIP_ERROR MeterIdentificationDelegate::SetProtocolVersion(const DataModel::Nullable<CharSpan> & newValue)
{
    if (NullableCharSpanCompare(newValue, mProtocolVersion))
    {
        return CHIP_NO_ERROR;
    }

    const CHIP_ERROR ret = NullableCharSpanCopy(newValue, mProtocolVersion);
    if (CHIP_NO_ERROR == ret)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, ProtocolVersion::Id);
    }
    return ret;
}

CHIP_ERROR MeterIdentificationDelegate::SetPowerThreshold(const DataModel::Nullable<Structs::PowerThresholdStruct::Type> & newValue)
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

        if ((newValue.Value().powerThreshold.HasValue() && INT64_MIN == newValue.Value().powerThreshold.Value()) ||
            (newValue.Value().apparentPowerThreshold.HasValue() && INT64_MIN == newValue.Value().apparentPowerThreshold.Value()) ||
            (!newValue.Value().powerThresholdSource.IsNull() && static_cast<std::underlying_type<PowerThresholdSourceEnum>::type>(
                PowerThresholdSourceEnum::kUnknownEnumValue) <= static_cast<std::underlying_type<PowerThresholdSourceEnum>::type>(
                    newValue.Value().powerThresholdSource.Value())))
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        mPowerThreshold.SetNonNull(newValue.Value());
    }

    MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThreshold::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::JsonToPowerThreshold(const Json::Value & root, Structs::PowerThresholdStruct::Type & value)
{
    Json::Value t;
    if (root.isMember("PowerThreshold"))
    {
        t = root.get("PowerThreshold", Json::Value());
        if (t.empty() || !t.isInt64())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.powerThreshold = Optional<int64_t>(t.asUInt());
    }

    if (root.isMember("ApparentPowerThreshold"))
    {
        t = root.get("ApparentPowerThreshold", Json::Value());
        if (t.empty() || !t.isInt64())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.apparentPowerThreshold = Optional<int64_t>(t.asUInt());
    }

    if (root.isMember("PowerThresholdSource"))
    {
        t = root.get("PowerThresholdSource", Json::Value());
        if (t.empty() || !t.isUInt())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.powerThresholdSource.SetNonNull(static_cast<PowerThresholdSourceEnum>(t.asUInt()));
    }

    return CHIP_NO_ERROR;
}

bool MeterIdentificationDelegate::NullableCharSpanCompare(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b)
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

CHIP_ERROR MeterIdentificationDelegate::NullableCharSpanCopy(const DataModel::Nullable<CharSpan> & src, DataModel::Nullable<CharSpan> & dst)
{
    const size_t len = src.IsNull() ? 0 : src.Value().size();
    if (64 < len)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!dst.IsNull())
    {
        chip::Platform::MemoryFree((void *) dst.Value().data());
        dst.SetNull();
    }

    if (!src.IsNull())
    {
        char * str = (char *) chip::Platform::MemoryAlloc(len + 1);
        strncpy(str, src.Value().data(), len);
        str[len] = 0;
        dst = MakeNullable(CharSpan::fromCharString(str));
    }

    return CHIP_NO_ERROR;
}

static std::unique_ptr<MeterIdentificationDelegate> gMIDelegate;
static std::unique_ptr<MeterIdentificationInstance> gMIInstance;
static BitMask<Feature> gMIFeature = BitMask<Feature, uint32_t>(Feature::kPowerThreshold);

void emberAfMeterIdentificationClusterInitCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(Zcl, "emberAfMeterIdentificationClusterInitCallback %d", (int)endpointId);

    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gMIInstance);

    gMIDelegate = std::make_unique<MeterIdentificationDelegate>();
    if (gMIDelegate)
    {
        gMIDelegate->Init();

        gMIInstance = std::make_unique<MeterIdentificationInstance>(endpointId, *gMIDelegate, gMIFeature);

        gMIInstance->Init();
    }
}

MeterIdentificationDelegate * chip::app::Clusters::MeterIdentification::GetDelegate()
{
    return &(*gMIDelegate);
}

namespace chip::app::Clusters::MeterIdentification {

void RestartServer(uint32_t features)
{
    gMIFeature = static_cast<BitMask<Feature>>(features);

    VerifyOrDie(gMIInstance);
    gMIInstance.reset();

    gMIInstance = std::make_unique<MeterIdentificationInstance>(1, *gMIDelegate, gMIFeature);
    gMIInstance->Init();
}

}