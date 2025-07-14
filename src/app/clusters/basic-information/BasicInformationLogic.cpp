/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/basic-information/BasicInformationLogic.h>

#include <app/InteractionModelEngine.h>
#include <clusters/BasicInformation/Enums.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::BasicInformation;

namespace chip {
namespace app {
namespace Clusters {
namespace {

static_assert(sizeof(bool) == 1, "I/O assumption for backwards compatibility");

void LogIfReadError(AttributeId attributeId, CHIP_ERROR err)
{
    VerifyOrReturn(err != CHIP_NO_ERROR);
    VerifyOrReturn(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    ChipLogError(Zcl, "BasicInformation: failed to load attribute " ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueMEI(attributeId), err.Format());
}

} // namespace

BasicInformationLogic & BasicInformationLogic::Instance()
{
    static BasicInformationLogic sInstance;
    return sInstance;
}

CHIP_ERROR BasicInformationLogic::Init(AttributePersistenceProvider & storage)
{
    {
        Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
        MutableByteSpan labelSpan = labelBuffer.RawBuffer();

        LogIfReadError(Attributes::NodeLabel::Id,
                       storage.ReadValue({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelSpan));

        if (!Storage::ShortPascalString::IsValid({ mNodeLabelBuffer, labelSpan.size() }))
        {
            // invalid value
            labelBuffer.SetValue(""_span);
        }
    }

    {
        MutableByteSpan localConfigBytes(reinterpret_cast<uint8_t *>(&mLocalConfigDisabled), sizeof(mLocalConfigDisabled));
        LogIfReadError(Attributes::LocalConfigDisabled::Id,
                       storage.ReadValue({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, localConfigBytes));

        if (localConfigBytes.size() == 0)
        {
            // invalid value
            mLocalConfigDisabled = false;
        }
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BasicInformationLogic::SetLocalConfigDisabled(bool value, AttributePersistenceProvider & storage)
{
    mLocalConfigDisabled = value;
    return storage.WriteValue({ kRootEndpointId, BasicInformation::Id, Attributes::LocalConfigDisabled::Id },
                              { reinterpret_cast<const uint8_t *>(&value), sizeof(value) });
}

DataModel::ActionReturnStatus BasicInformationLogic::SetNodeLabel(CharSpan label, AttributePersistenceProvider & storage)
{
    Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
    VerifyOrReturnError(labelBuffer.SetValue(label), Protocols::InteractionModel::Status::ConstraintError);
    return storage.WriteValue({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelBuffer.RawValidData());
}

DataModel::ActionReturnStatus BasicInformationLogic::SetLocation(CharSpan location)
{
    VerifyOrReturnError(location.size() == kFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
    return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
}

} // namespace Clusters
} // namespace app
} // namespace chip
