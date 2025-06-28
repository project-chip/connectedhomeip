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

void LogIfReadError(AttributeId attributeId, CHIP_ERROR err) {}

} // namespace

BasicInformationLogic & BasicInformationLogic::Instance()
{
    static BasicInformationLogic sInstance;
    return sInstance;
}

CHIP_ERROR BasicInformationLogic::Init(Storage::AttributeStorage & storage)
{
    Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
    LogIfReadError(Attributes::NodeLabel::Id,
                   storage.Read({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelBuffer));

    LogIfReadError(Attributes::LocalConfigDisabled::Id,
                   storage.Read({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id },
                                Storage::AttributeStorage::Buffer::Primitive(mLocalConfigDisabled)));

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BasicInformationLogic::SetLocalConfigDisabled(bool value, Storage::AttributeStorage & storage)
{
    mLocalConfigDisabled = value;
    return storage.Write({ kRootEndpointId, BasicInformation::Id, Attributes::LocalConfigDisabled::Id },
                         Storage::AttributeStorage::Value::Primitive(value));
}

DataModel::ActionReturnStatus BasicInformationLogic::SetNodeLabel(CharSpan label, Storage::AttributeStorage & storage)
{
    Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
    VerifyOrReturnError(labelBuffer.SetValue(label), Protocols::InteractionModel::Status::ConstraintError);
    return storage.Write({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelBuffer);
}

DataModel::ActionReturnStatus BasicInformationLogic::SetLocation(CharSpan location)
{
    VerifyOrReturnError(location.size() == kFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
    return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
}

} // namespace Clusters
} // namespace app
} // namespace chip
