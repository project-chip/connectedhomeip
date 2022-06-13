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
 * @brief Implementation for the User Label Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UserLabel;
using namespace chip::app::Clusters::UserLabel::Attributes;

namespace {

class UserLabelAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the User Label cluster on all endpoints.
    UserLabelAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), UserLabel::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadLabelList(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteLabelList(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
};

UserLabelAttrAccess gAttrAccess;

CHIP_ERROR UserLabelAttrAccess::ReadLabelList(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::UserLabelIterator * it = provider->IterateUserLabel(endpoint);

        if (it)
        {
            err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                UserLabel::Structs::LabelStruct::Type userlabel;

                while (it->Next(userlabel))
                {
                    ReturnErrorOnFailure(encoder.Encode(userlabel));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
        }
        else
        {
            err = aEncoder.EncodeEmptyList();
        }
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }

    return err;
}

CHIP_ERROR UserLabelAttrAccess::WriteLabelList(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    EndpointId endpoint                        = aPath.mEndpointId;
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    if (!aPath.IsListItemOperation())
    {
        DeviceLayer::AttributeList<Structs::LabelStruct::Type, DeviceLayer::kMaxUserLabelListLength> labelList;
        LabelList::TypeInfo::DecodableType decodablelist;

        ReturnErrorOnFailure(aDecoder.Decode(decodablelist));

        auto iter = decodablelist.begin();
        while (iter.Next())
        {
            auto & entry = iter.GetValue();
            ReturnErrorOnFailure(labelList.add(entry));
        }
        ReturnErrorOnFailure(iter.GetStatus());

        return provider->SetUserLabelList(endpoint, labelList);
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        Structs::LabelStruct::DecodableType entry;
        ReturnErrorOnFailure(aDecoder.Decode(entry));
        return provider->AppendUserLabel(endpoint, entry);
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR UserLabelAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == UserLabel::Id);

    switch (aPath.mAttributeId)
    {
    case LabelList::Id:
        return ReadLabelList(aPath.mEndpointId, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR UserLabelAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == UserLabel::Id);

    switch (aPath.mAttributeId)
    {
    case LabelList::Id:
        return WriteLabelList(aPath, aDecoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

void MatterUserLabelPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
