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

CHIP_ERROR BasicInformationLogic::Init()
{
    // TODO: implement
    //
    // Some prototype:
    //
    // ReadDestination data(mNodeLabelBuffer);
    //
    // ReturnErrorOnFailure(mPersistence.ReadAttribute({kRootEndpointId, BasicInformation::Id, NodeLabel::Id}, data));
    // mNodeLabelSize = data.size();
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BasicInformationLogic::SetNodeLabel(CharSpan label)
{
    // TODO:
    //   Label MUST BE PERSISTED across reboots
    //   Label MUST BE STORED COMPATIBLE !
    VerifyOrReturnError(label.size() <= sizeof(mNodeLabelBuffer), Protocols::InteractionModel::Status::ConstraintError);
    memcpy(mNodeLabelBuffer, label.data(), label.size());
    mNodeLabelSize = static_cast<uint8_t>(label.size()); // we know that label size is at most 32

    // TODO: implement
    //
    // Some prototype:
    //
    //   return mPersistence.WriteAttribute(
    //     {kRootEndpointId, BasicInformation::Id, NodeLabel::Id},
    //     AttributeData(label)
    //   );

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BasicInformationLogic::SetLocation(CharSpan location)
{
    VerifyOrReturnError(location.size() == kFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
    return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
}

} // namespace Clusters
} // namespace app
} // namespace chip
