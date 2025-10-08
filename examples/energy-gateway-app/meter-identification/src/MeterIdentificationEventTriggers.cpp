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

#include <MeterIdentificationInstance.h>
#include <app/clusters/meter-identification-server/MeterIdentificationTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;

namespace {

class OldMeterIdentificationAttributes
{
private:
    Instance * mInstance                           = nullptr;
    static constexpr size_t kMaximumStringSize     = 64;
    char mPointOfDeliveryBuf[kMaximumStringSize]   = {};
    char mMeterSerialNumberBuf[kMaximumStringSize] = {};
    char mProtocolVersionBuf[kMaximumStringSize]   = {};
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> mPowerThreshold;

    struct DataPresets_s
    {
        DataModel::Nullable<MeterTypeEnum> meterType;
        DataModel::Nullable<CharSpan> pointOfDelivery;
        DataModel::Nullable<CharSpan> meterSerialNumber;
        DataModel::Nullable<CharSpan> protocolVersion;
        DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> powerThreshold;
    };

    static constexpr uint8_t kMaxPresetItems = 2u;
    uint8_t mPresetsIdx;

    const DataPresets_s TestsDataPresets[kMaxPresetItems] = {
        { .meterType         = DataModel::MakeNullable(MeterTypeEnum::kUtility),
          .pointOfDelivery   = DataModel::MakeNullable(CharSpan::fromCharString("Test delivery point")),
          .meterSerialNumber = DataModel::MakeNullable(CharSpan::fromCharString("TST-123456789")),
          .protocolVersion   = DataModel::MakeNullable(CharSpan::fromCharString("1.2.3")),
          .powerThreshold    = DataModel::MakeNullable(Globals::Structs::PowerThresholdStruct::Type(
              { Optional<int64_t>(2400000), Optional<int64_t>(120), Globals::PowerThresholdSourceEnum::kContract })) },
        { .meterType         = DataModel::MakeNullable(MeterTypeEnum::kPrivate),
          .pointOfDelivery   = DataModel::MakeNullable(CharSpan::fromCharString("New delivery point")),
          .meterSerialNumber = DataModel::MakeNullable(CharSpan::fromCharString("NEW-987654321")),
          .protocolVersion   = DataModel::MakeNullable(CharSpan::fromCharString("3.4.5")),
          .powerThreshold    = DataModel::MakeNullable(Globals::Structs::PowerThresholdStruct::Type(
              { Optional<int64_t>(4800000), Optional<int64_t>(240), Globals::PowerThresholdSourceEnum::kRegulator })) }
    };

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
            const size_t len = newValue.IsNull()               ? 0
                : newValue.Value().size() < kMaximumStringSize ? newValue.Value().size()
                                                               : kMaximumStringSize;
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
            const size_t len = newValue.IsNull()               ? 0
                : newValue.Value().size() < kMaximumStringSize ? newValue.Value().size()
                                                               : kMaximumStringSize;
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
            const size_t len = newValue.IsNull()               ? 0
                : newValue.Value().size() < kMaximumStringSize ? newValue.Value().size()
                                                               : kMaximumStringSize;
            memmove(mProtocolVersionBuf, newValue.Value().data(), len);
            mProtocolVersion = DataModel::MakeNullable(CharSpan(mProtocolVersionBuf, len));
        }
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

    void UpdAttrsByPresetIdx()
    {
        mInstance->SetMeterType(TestsDataPresets[mPresetsIdx].meterType);
        mInstance->SetPointOfDelivery(TestsDataPresets[mPresetsIdx].pointOfDelivery);
        mInstance->SetMeterSerialNumber(TestsDataPresets[mPresetsIdx].meterSerialNumber);
        mInstance->SetProtocolVersion(TestsDataPresets[mPresetsIdx].protocolVersion);
        mInstance->SetPowerThreshold(TestsDataPresets[mPresetsIdx].powerThreshold);

        mPresetsIdx = !mPresetsIdx;
    }

public:
    OldMeterIdentificationAttributes() { mPresetsIdx = 0; };
    ~OldMeterIdentificationAttributes() = default;

    void Update()
    {
        if (!mInstance)
        {
            SaveAttributes();
        }

        UpdAttrsByPresetIdx();
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
