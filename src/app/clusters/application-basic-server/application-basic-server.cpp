/**
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
 * @brief Routines for the Application Basic plugin, the
 *server implementation of the Application Basic cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-basic-server/application-basic-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <list>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationBasic;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ApplicationBasic::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::ApplicationBasic::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Application Basic has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::ApplicationBasic::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class ApplicationBasicAttrAccess : public app::AttributeAccessInterface
{
public:
    ApplicationBasicAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::ApplicationBasic::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadVendorNameAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadVendorIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadApplicationNameAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadProductIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadApplicationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadStatusAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadApplicationVersionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadAllowedVendorListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ApplicationBasicAttrAccess gApplicationBasicAttrAccess;

CHIP_ERROR ApplicationBasicAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (isDelegateNull(delegate, endpoint))
    {
        if (aPath.mAttributeId == Attributes::AllowedVendorList::Id)
        {
            return aEncoder.EncodeEmptyList();
        }
        return CHIP_NO_ERROR;
    }

    switch (aPath.mAttributeId)
    {
    case app::Clusters::ApplicationBasic::Attributes::VendorName::Id: {
        return ReadVendorNameAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::VendorId::Id: {
        return ReadVendorIdAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::ApplicationName::Id: {
        return ReadApplicationNameAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::ProductId::Id: {
        return ReadProductIdAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::ApplicationApp::Id: {
        return ReadApplicationAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::ApplicationStatus::Id: {
        return ReadStatusAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::Id: {
        return ReadApplicationVersionAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::Id: {
        return ReadAllowedVendorListAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadVendorNameAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    chip::CharSpan vendorName = delegate->HandleGetVendorName();
    return aEncoder.Encode(vendorName);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadVendorIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint16_t vendorId = delegate->HandleGetVendorId();
    return aEncoder.Encode(vendorId);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationNameAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    chip::CharSpan applicationName = delegate->HandleGetApplicationName();
    return aEncoder.Encode(applicationName);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadProductIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint16_t vendorId = delegate->HandleGetProductId();
    return aEncoder.Encode(vendorId);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    Structs::ApplicationBasicApplication::Type application = delegate->HandleGetApplication();
    return aEncoder.Encode(application);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadStatusAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    ApplicationStatusEnum status = delegate->HandleGetStatus();
    return aEncoder.Encode(status);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationVersionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    chip::CharSpan applicationVersion = delegate->HandleGetApplicationVersion();
    return aEncoder.Encode(applicationVersion);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadAllowedVendorListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    std::list<uint16_t> allowedVendorList = delegate->HandleGetAllowedVendorList();
    return aEncoder.EncodeList([allowedVendorList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & allowedVendor : allowedVendorList)
        {
            ReturnErrorOnFailure(encoder.Encode(allowedVendor));
        }
        return CHIP_NO_ERROR;
    });
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterApplicationBasicPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gApplicationBasicAttrAccess);
}
