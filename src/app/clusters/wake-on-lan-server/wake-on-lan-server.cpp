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
 * @brief Routines for the Wake on LAN plugin, the
 *server implementation of the Wake on LAN cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/wake-on-lan-server/wake-on-lan-delegate.h>
#include <app/clusters/wake-on-lan-server/wake-on-lan-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WakeOnLan;

static constexpr size_t kWakeOnLanDelegateTableSize =
    MATTER_DM_WAKE_ON_LAN_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kWakeOnLanDelegateTableSize <= kEmberInvalidEndpointIndex, "WakeOnLan Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::WakeOnLan::Delegate;

namespace {

Delegate * gDelegateTable[kWakeOnLanDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WakeOnLan::Id, MATTER_DM_WAKE_ON_LAN_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kWakeOnLanDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "WakeOnLan has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WakeOnLan {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WakeOnLan::Id, MATTER_DM_WAKE_ON_LAN_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kWakeOnLanDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace WakeOnLan
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class WakeOnLanAttrAccess : public app::AttributeAccessInterface
{
public:
    WakeOnLanAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::WakeOnLan::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadMacAddressAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

WakeOnLanAttrAccess gWakeOnLanAttrAccess;

CHIP_ERROR WakeOnLanAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (isDelegateNull(delegate, endpoint))
    {
        return CHIP_NO_ERROR;
    }

    switch (aPath.mAttributeId)
    {
    case app::Clusters::WakeOnLan::Attributes::MACAddress::Id: {
        return ReadMacAddressAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WakeOnLanAttrAccess::ReadMacAddressAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetMacAddress(aEncoder);
}

} // anonymous namespace

void MatterWakeOnLanPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gWakeOnLanAttrAccess);
}
