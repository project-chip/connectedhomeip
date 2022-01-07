/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Fixed Label Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FixedLabel;
using namespace chip::app::Clusters::FixedLabel::Attributes;

namespace {

class FixedLabelAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Fixed Label cluster on all endpoints.
    FixedLabelAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), FixedLabel::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadLabelList(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR FixedLabelAttrAccess::ReadLabelList(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::AttributeList<app::Clusters::FixedLabel::Structs::LabelStruct::Type, DeviceLayer::kMaxFixedLabels> labelList;

    if (DeviceLayer::PlatformMgr().GetFixedLabelList(endpoint, labelList) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&labelList](const auto & encoder) -> CHIP_ERROR {
            for (auto label : labelList)
            {
                ReturnErrorOnFailure(encoder.Encode(label));
            }

            return CHIP_NO_ERROR;
        });
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }

    return err;
}

FixedLabelAttrAccess gAttrAccess;

CHIP_ERROR FixedLabelAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == FixedLabel::Id);

    switch (aPath.mAttributeId)
    {
    case LabelList::Id: {
        return ReadLabelList(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

void MatterFixedLabelPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
