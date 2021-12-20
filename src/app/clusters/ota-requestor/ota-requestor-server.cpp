/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* This file contains the glue code for passing the incoming OTA Requestor cluster commands
 * to the OTA Requestor object that handles them
 */

#include <app/AttributeAccessInterface.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <platform/OTARequestorInterface.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;

namespace {

class OtaSoftwareUpdateRequestorAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OTA Requestor Cluster on all endpoints.
    OtaSoftwareUpdateRequestorAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), OtaSoftwareUpdateRequestor::Id)
    {}

    // TODO: Implement Read/Write for OtaSoftwareUpdateRequestorAttrAccess
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};

OtaSoftwareUpdateRequestorAttrAccess gAttrAccess;

CHIP_ERROR OtaSoftwareUpdateRequestorAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::DefaultOtaProviders::Id:
        return aEncoder.Encode(DataModel::List<uint8_t>());
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaSoftwareUpdateRequestorAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::DefaultOtaProviders::Id: {
        DataModel::DecodableList<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));
        // Ignore the list for now
        break;
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace

bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    EmberAfStatus status;
    chip::OTARequestorInterface * requestor = chip::GetRequestorInstance();

    if (requestor != nullptr)
    {
        status = requestor->HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterOtaSoftwareUpdateRequestorPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
