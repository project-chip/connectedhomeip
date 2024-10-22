/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "BridgedDeviceBasicInformationImpl.h"

#include "BridgedDeviceManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <app/AttributeAccessInterfaceRegistry.h>

static constexpr unsigned kBridgedDeviceBasicInformationClusterRevision = 4;

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;

CHIP_ERROR BridgedDeviceBasicInformationImpl::Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder)
{
    // Registration is done for the bridged device basic information only
    VerifyOrDie(path.mClusterId == app::Clusters::BridgedDeviceBasicInformation::Id);

    BridgedDevice * dev = BridgeDeviceMgr().GetDevice(path.mEndpointId);
    VerifyOrReturnError(dev != nullptr, CHIP_ERROR_NOT_FOUND);

    switch (path.mAttributeId)
    {
    case BasicInformation::Attributes::Reachable::Id:
        encoder.Encode(dev->IsReachable());
        break;
    case BasicInformation::Attributes::NodeLabel::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().nodeLabel.c_str()));
        break;
    case BasicInformation::Attributes::ClusterRevision::Id:
        encoder.Encode(kBridgedDeviceBasicInformationClusterRevision);
        break;
    case BasicInformation::Attributes::FeatureMap::Id: {
        BitMask<Clusters::BridgedDeviceBasicInformation::Feature> features;
        features.Set(Clusters::BridgedDeviceBasicInformation::Feature::kBridgedICDSupport, dev->IsIcd());
        encoder.Encode(features);
    }
    break;
    case BasicInformation::Attributes::UniqueID::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().uniqueId.c_str()));
        break;
    case BasicInformation::Attributes::VendorName::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().vendorName.c_str()));
        break;
    case BasicInformation::Attributes::VendorID::Id:
        encoder.Encode(dev->GetBridgedAttributes().vendorId);
        break;
    case BasicInformation::Attributes::ProductName::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().productName.c_str()));
        break;
    case BasicInformation::Attributes::ProductID::Id:
        encoder.Encode(dev->GetBridgedAttributes().productId);
        break;
    case BasicInformation::Attributes::HardwareVersion::Id:
        encoder.Encode(dev->GetBridgedAttributes().hardwareVersion);
        break;
    case BasicInformation::Attributes::HardwareVersionString::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().hardwareVersionString.c_str()));
        break;
    case BasicInformation::Attributes::SoftwareVersion::Id:
        encoder.Encode(dev->GetBridgedAttributes().softwareVersion);
        break;
    case BasicInformation::Attributes::SoftwareVersionString::Id:
        encoder.Encode(CharSpan::fromCharString(dev->GetBridgedAttributes().softwareVersionString.c_str()));
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BridgedDeviceBasicInformationImpl::Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    VerifyOrDie(path.mClusterId == app::Clusters::BridgedDeviceBasicInformation::Id);

    BridgedDevice * dev = BridgeDeviceMgr().GetDevice(path.mEndpointId);
    VerifyOrReturnError(dev != nullptr, CHIP_ERROR_NOT_FOUND);

    if (!dev->IsReachable())
    {
        return CHIP_ERROR_NOT_CONNECTED;
    }

    ChipLogProgress(NotSpecified, "Bridged device basic information attempt to write attribute: ep=%d", path.mAttributeId);

    // nothing writable right now ...

    return CHIP_ERROR_INVALID_ARGUMENT;
}
