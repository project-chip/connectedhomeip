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

BasicInformationLogic & BasicInformationLogic::Instance()
{
    static BasicInformationLogic sInstance;
    return sInstance;
}

CHIP_ERROR BasicInformationLogic::Init(Storage::AttributeStorage & storage)
{
    Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
    CHIP_ERROR err = storage.Read({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelBuffer);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
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
