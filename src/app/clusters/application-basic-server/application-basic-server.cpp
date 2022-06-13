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
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

#include <list>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationBasic;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static constexpr size_t kApplicationBasicDelegateTableSize =
    EMBER_AF_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ApplicationBasic::Delegate;

namespace {

Delegate * gDelegateTable[kApplicationBasicDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "ApplicationBasic returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetApplicationBasicDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "ApplicationBasic NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::ApplicationBasic::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Application Basic has no delegate set for endpoint:%u", endpoint);
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
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ApplicationBasic::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_APPLICATION_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

CHIP_ERROR Delegate::HandleGetApplication(app::AttributeValueEncoder & aEncoder)
{
    ApplicationBasicApplicationType application;
    application.catalogVendorId = mCatalogVendorApp.catalogVendorId;
    application.applicationId   = CharSpan(mCatalogVendorApp.applicationId, strlen(mCatalogVendorApp.applicationId));
    return aEncoder.Encode(application);
}

bool Delegate::Matches(ApplicationBasicApplication match)
{
    std::string appId(match.applicationId.data(), match.applicationId.size());
    CatalogVendorApp matchApp(match.catalogVendorId, appId.c_str());
    return mCatalogVendorApp.Matches(&matchApp);
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
    ApplicationBasicAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), ApplicationBasic::Id) {}

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
    case chip::app::Clusters::ApplicationBasic::Attributes::VendorName::Id: {
        return ReadVendorNameAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::VendorID::Id: {
        return ReadVendorIdAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::Id: {
        return ReadApplicationNameAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::ProductID::Id: {
        return ReadProductIdAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::Application::Id: {
        return ReadApplicationAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::Status::Id: {
        return ReadStatusAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::Id: {
        return ReadApplicationVersionAttribute(aEncoder, delegate);
    }
    case chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::Id: {
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
    return delegate->HandleGetVendorName(aEncoder);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadVendorIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint16_t vendorId = delegate->HandleGetVendorId();
    return aEncoder.Encode(vendorId);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationNameAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetApplicationName(aEncoder);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadProductIdAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint16_t vendorId = delegate->HandleGetProductId();
    return aEncoder.Encode(vendorId);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetApplication(aEncoder);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadStatusAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    ApplicationStatusEnum status = delegate->HandleGetStatus();
    return aEncoder.Encode(status);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadApplicationVersionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetApplicationVersion(aEncoder);
}

CHIP_ERROR ApplicationBasicAttrAccess::ReadAllowedVendorListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetAllowedVendorList(aEncoder);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterApplicationBasicPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gApplicationBasicAttrAccess);
}
