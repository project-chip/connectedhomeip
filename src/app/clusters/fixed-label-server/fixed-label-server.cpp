/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Fixed Label Server Cluster
 ***************************************************************************/

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

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::FixedLabelIterator * it = provider->IterateFixedLabel(endpoint);

        if (it)
        {
            err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                FixedLabel::Structs::LabelStruct::Type fixedlabel;

                while (it->Next(fixedlabel))
                {
                    ReturnErrorOnFailure(encoder.Encode(fixedlabel));
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

void MatterFixedLabelPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
