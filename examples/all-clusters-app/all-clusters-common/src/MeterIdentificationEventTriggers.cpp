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

#include <meter-identification-delegate.h>

#include <app/clusters/meter-identification-server/MeterIdentificationTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;

namespace {

class OldMeterIdentificationAttributes
{
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Structs::PowerThresholdStruct::Type> mPowerThreshold;

private:

    DataModel::Nullable<MeterIdentificationDelegate *> mDelegate;

    static void SetCharSpan(DataModel::Nullable<CharSpan> & charSpan, const DataModel::Nullable<CharSpan> && value)
    {
        if (!value.IsNull())
        {
            const size_t len = value.Value().size();
            char * str = (char *) chip::Platform::MemoryAlloc(len + 1);
            strncpy(str, value.Value().data(), len);
            str[len] = 0;
            charSpan = DataModel::MakeNullable(CharSpan::fromCharString(str));
        }
    }

    static void CleanCharSpan(DataModel::Nullable<CharSpan> & charSpan)
    {
        if (!charSpan.IsNull())
        {
            chip::Platform::MemoryFree((void *) charSpan.Value().data());
            charSpan.SetNull();
        }
    }

    static std::string IncrementString(const std::string &&string)
    {
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
                ++(*(ret.rbegin() + std::distance(string.rbegin(), rit)));
                break;
            }
        }

        return ret;
    }

    void SaveAttributes()
    {
        mDelegate.SetNonNull(GetDelegate());
        VerifyOrDieWithMsg(mDelegate.Value() != nullptr, AppServer, "Meter Identification Delegate is null");
        mMeterType = mDelegate.Value()->GetMeterType();
        SetCharSpan(mPointOfDelivery, mDelegate.Value()->GetPointOfDelivery());
        SetCharSpan(mMeterSerialNumber, mDelegate.Value()->GetMeterSerialNumber());
        SetCharSpan(mProtocolVersion, mDelegate.Value()->GetProtocolVersion());
        const auto && powerThreshold = mDelegate.Value()->GetPowerThreshold();
        if (!powerThreshold.IsNull())
        {
            mPowerThreshold.SetNonNull(powerThreshold.Value());
        }
    }

    void ClearAttributes()
    {
        CleanCharSpan(mPointOfDelivery);
        CleanCharSpan(mMeterSerialNumber);
        CleanCharSpan(mProtocolVersion);
        mMeterType.SetNull();
        mPowerThreshold.SetNull();
        mDelegate.SetNull();
    }

    void RestoreAttributes() const
    {
        if (!mDelegate.IsNull())
        {
            mDelegate.Value()->SetMeterType(mMeterType);
            mDelegate.Value()->SetPointOfDelivery(mPointOfDelivery);
            mDelegate.Value()->SetMeterSerialNumber(mMeterSerialNumber);
            mDelegate.Value()->SetProtocolVersion(mProtocolVersion);
            mDelegate.Value()->SetPowerThreshold(mPowerThreshold);
        }
    }

    void IncreaseAttributes()
    {
        if (mDelegate.Value()->GetMeterType().IsNull())
        {
            mDelegate.Value()->SetMeterType(DataModel::MakeNullable(static_cast<MeterTypeEnum>(0)));
        }
        else
        {
            mDelegate.Value()->SetMeterType(DataModel::MakeNullable(static_cast<MeterTypeEnum>(1 +
                static_cast<std::underlying_type<MeterTypeEnum>::type>(mDelegate.Value()->GetMeterType().Value()))));
        }

        if (mDelegate.Value()->GetPointOfDelivery().IsNull())
        {
            mDelegate.Value()->SetPointOfDelivery(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            mDelegate.Value()->SetPointOfDelivery(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                mDelegate.Value()->GetPointOfDelivery().Value().data()).c_str())));
        }

        if (mDelegate.Value()->GetMeterSerialNumber().IsNull())
        {
            mDelegate.Value()->SetMeterSerialNumber(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            mDelegate.Value()->SetMeterSerialNumber(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                mDelegate.Value()->GetMeterSerialNumber().Value().data()).c_str())));
        }

        if (mDelegate.Value()->GetProtocolVersion().IsNull())
        {
            mDelegate.Value()->SetProtocolVersion(DataModel::MakeNullable(CharSpan::fromCharString("")));
        }
        else
        {
            mDelegate.Value()->SetProtocolVersion(DataModel::MakeNullable(CharSpan::fromCharString(IncrementString(
                mDelegate.Value()->GetProtocolVersion().Value().data()).c_str())));
        }

        if (mDelegate.Value()->GetPowerThreshold().IsNull())
        {
            mDelegate.Value()->SetPowerThreshold(DataModel::MakeNullable((Structs::PowerThresholdStruct::Type){Optional<int64_t>(0),
                Optional<int64_t>(0), static_cast<PowerThresholdSourceEnum>(0)}));
        }
        else
        {
            auto powerThreshold = mDelegate.Value()->GetPowerThreshold();
            ++powerThreshold.Value().powerThreshold.Value();
            ++powerThreshold.Value().apparentPowerThreshold.Value();
            powerThreshold.Value().powerThresholdSource.Value() = static_cast<PowerThresholdSourceEnum>(1 +
                static_cast<std::underlying_type<PowerThresholdSourceEnum>::type>(powerThreshold.Value().powerThresholdSource.Value()));
            mDelegate.Value()->SetPowerThreshold(std::move(powerThreshold));
        }
    }

public:

    void Update()
    {
        if (mDelegate.IsNull())
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
