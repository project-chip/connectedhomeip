/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <meter-identification-instance.h>
#include <app/clusters/meter-identification-server/MeterIdentificationTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;

namespace {

class OldMeterIdentificationAttributes
{
private:

    Instance * mInstance = nullptr;
    static constexpr size_t kMaximumStringSize = 64;
    char mPointOfDeliveryBuf[kMaximumStringSize] = {};
    char mMeterSerialNumberBuf[kMaximumStringSize] = {};
    char mProtocolVersionBuf[kMaximumStringSize] = {};
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> mPowerThreshold;

    static bool NullableCharSpansEqual(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b)
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

    void SavePointOfDelivery(const DataModel::Nullable<CharSpan> & newValue)
    {
        if (NullableCharSpansEqual(newValue, mPointOfDelivery))
        {
            return;
        }

        if (!mPointOfDelivery.IsNull())
        {
            mPointOfDelivery.SetNull();
        }

        if (!newValue.IsNull())
        {
            const size_t len = newValue.IsNull() ? 0 : newValue.Value().size() < kMaximumStringSize ?
                newValue.Value().size() : kMaximumStringSize;
            memmove(mPointOfDeliveryBuf, newValue.Value().data(), len);
            mPointOfDelivery = DataModel::MakeNullable(CharSpan(mPointOfDeliveryBuf, len));
        }
    }

    void SaveMeterSerialNumber(const DataModel::Nullable<CharSpan> & newValue)
    {
        if (NullableCharSpansEqual(newValue, mMeterSerialNumber))
        {
            return;
        }

        if (!mMeterSerialNumber.IsNull())
        {
            mMeterSerialNumber.SetNull();
        }

        if (!newValue.IsNull())
        {
            const size_t len = newValue.IsNull() ? 0 : newValue.Value().size() < kMaximumStringSize ?
                newValue.Value().size() : kMaximumStringSize;
            memmove(mMeterSerialNumberBuf, newValue.Value().data(), len);
            mMeterSerialNumber = DataModel::MakeNullable(CharSpan(mMeterSerialNumberBuf, len));
        }
    }

    void SaveProtocolVersion(const DataModel::Nullable<CharSpan> & newValue)
    {
        if (NullableCharSpansEqual(newValue, mProtocolVersion))
        {
            return;
        }

        if (!mProtocolVersion.IsNull())
        {
            mProtocolVersion.SetNull();
        }

        if (!newValue.IsNull())
        {
            const size_t len = newValue.IsNull() ? 0 : newValue.Value().size() < kMaximumStringSize ?
                newValue.Value().size() : kMaximumStringSize;
            memmove(mProtocolVersionBuf, newValue.Value().data(), len);
            mProtocolVersion = DataModel::MakeNullable(CharSpan(mProtocolVersionBuf, len));
        }
    }

    /**
     * Lexicographically increments a string.
     *
     * @param string A string to lexicographically increment
     */
    static std::string IncrementString(const std::string &&string)
    {
        // The lexicographically smallest and largest characters
        constexpr char minC = ' ', maxC = '~';

        std::string ret{minC};

        auto rit = string.rbegin();
        while (rit != string.rend())
        {
            if (maxC == *rit)
            {
                ++rit;
                if (rit == string.rend())
                {
                    ret = string + ret;
                    break;
                }
            }
            else
            {
                ret = string;
                ++(*(ret.rbegin() + distance(string.rbegin(), rit)));
                break;
            }
        }

        return ret;
    }

    void SaveAttributes()
    {
        mInstance = GetInstance();
        VerifyOrDieWithMsg(mInstance, AppServer, "Meter Identification instance is null");
        mMeterType = mInstance->GetMeterType();
        SavePointOfDelivery(mInstance->GetPointOfDelivery());
        SaveMeterSerialNumber(mInstance->GetMeterSerialNumber());
        SaveProtocolVersion(mInstance->GetProtocolVersion());
        mPowerThreshold = mInstance->GetPowerThreshold();
    }

    void ClearAttributes()
    {
        mPointOfDelivery.SetNull();
        mMeterSerialNumber.SetNull();
        mProtocolVersion.SetNull();
        mMeterType.SetNull();
        mPowerThreshold.SetNull();
        mInstance = nullptr;
    }

    void RestoreAttributes() const
    {
        if (mInstance)
        {
            mInstance->SetMeterType(mMeterType);
            mInstance->SetPointOfDelivery(mPointOfDelivery);
            mInstance->SetMeterSerialNumber(mMeterSerialNumber);
            mInstance->SetProtocolVersion(mProtocolVersion);
            mInstance->SetPowerThreshold(mPowerThreshold);
        }
    }

    void IncreaseAttributes()
    {
        VerifyOrDieWithMsg(mInstance, AppServer, "Meter Identification instance is null");
        if (mInstance->GetMeterType().IsNull())
        {
            mInstance->SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kUtility));
        }
        else
        {
            mInstance->SetMeterType(DataModel::MakeNullable(static_cast<MeterTypeEnum>(1 +
                to_underlying(mInstance->GetMeterType().Value()))));
        }

        if (mInstance->GetPointOfDelivery().IsNull())
        {
            mInstance->SetPointOfDelivery(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            const auto value = mInstance->GetPointOfDelivery().Value();
            mInstance->SetPointOfDelivery(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                std::string(value.data(), value.size())).c_str())));
        }

        if (mInstance->GetMeterSerialNumber().IsNull())
        {
            mInstance->SetMeterSerialNumber(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            const auto value = mInstance->GetMeterSerialNumber().Value();
            mInstance->SetMeterSerialNumber(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                std::string(value.data(), value.size())).c_str())));
        }

        if (mInstance->GetProtocolVersion().IsNull())
        {
            mInstance->SetProtocolVersion(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            const auto value = mInstance->GetProtocolVersion().Value();
            mInstance->SetProtocolVersion(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                std::string(value.data(), value.size())).c_str())));
        }

        if (mInstance->GetPowerThreshold().IsNull())
        {
            mInstance->SetPowerThreshold(DataModel::MakeNullable(Globals::Structs::PowerThresholdStruct::Type({Optional<int64_t>(0),
                Optional<int64_t>(0), Globals::PowerThresholdSourceEnum::kContract})));
        }
        else
        {
            auto powerThreshold = mInstance->GetPowerThreshold();
            ++powerThreshold.Value().powerThreshold.Value();
            ++powerThreshold.Value().apparentPowerThreshold.Value();
            powerThreshold.Value().powerThresholdSource.Value() = static_cast<Globals::PowerThresholdSourceEnum>(1 +
                to_underlying(powerThreshold.Value().powerThresholdSource.Value()));
            mInstance->SetPowerThreshold(std::move(powerThreshold));
        }
    }

public:

    void Update()
    {
        if (!mInstance)
        {
            SaveAttributes();
        }

        IncreaseAttributes();
    }

    void Clear()
    {
        RestoreAttributes();
        ClearAttributes();
    }
};

OldMeterIdentificationAttributes mOldMeterIdentificationAttributes;
} // namespace

bool HandleMeterIdentificationTestEventTrigger(uint64_t eventTrigger)
{
    MeterIdentificationTrigger trigger = static_cast<MeterIdentificationTrigger>(eventTrigger);

    switch (trigger)
    {
    case MeterIdentificationTrigger::kAttributesValueUpdate:
        ChipLogProgress(Support, "[MTRID-Test-Event] => Attributes value update");
        mOldMeterIdentificationAttributes.Update();
        break;
    case MeterIdentificationTrigger::kAttributesValueUpdateClear:
        ChipLogProgress(Support, "[MTRID-Test-Event] => Attributes value clear");
        mOldMeterIdentificationAttributes.Clear();
        break;
    default:
        return false;
    }

    return true;
}