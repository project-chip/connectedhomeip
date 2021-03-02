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

#include "Mdns.h"

#include <core/Optional.h>
#include <mdns/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>
#include <transport/AdminPairingTable.h>
#include <transport/PASESession.h>

#include "Server.h"

namespace chip {
namespace app {
namespace Mdns {

namespace {

using namespace chip;

NodeId GetCurrentNodeId()
{
    // TODO: once operational credentials are implemented, node ID should be read from them
    if (!DeviceLayer::ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogError(Discovery, "Device not fully provisioned. Node ID unknown.");
        return chip::kTestDeviceNodeId;
    }

    // Admin pairings should have been persisted and should be loadable

    // TODO: once multi-admin is decided, figure out if a single node id
    // is sufficient or if we need multi-node-id advertisement. Existing
    // mdns advertises a single node id as parameter.

    // Search for one admin pariing and use its node id.
    auto pairing = GetGlobalAdminPairingTable().cbegin();
    if (pairing != GetGlobalAdminPairingTable().cend())
    {
        ChipLogProgress(Discovery, "Found admin paring for admin %" PRIX64 ", node %" PRIX64, pairing->GetAdminId(),
                        pairing->GetNodeId());
        return pairing->GetNodeId();
    }

    ChipLogError(Discovery, "Failed to find a valid admin pairing. Node ID unknown");
    return chip::kTestDeviceNodeId;
}

} // namespace

/// Set MDNS operational advertisement
CHIP_ERROR AdvertiseOperational()
{

    uint64_t fabricId;

    if (DeviceLayer::ConfigurationMgr().GetFabricId(fabricId) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Fabric ID not known. Using a default");
        fabricId = 5544332211;
    }

    const auto advertiseParameters = chip::Mdns::OperationalAdvertisingParameters()
                                         .SetFabricId(fabricId)
                                         .SetNodeId(GetCurrentNodeId())
                                         .SetPort(CHIP_PORT)
                                         .EnableIpV4(true);

    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

    ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));

    return mdnsAdvertiser.Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
}

/// Set MDNS commisioning advertisement
CHIP_ERROR AdvertiseCommisioning()
{

    auto advertiseParameters = chip::Mdns::CommissionAdvertisingParameters().SetPort(CHIP_PORT).EnableIpV4(true);

    uint16_t value;
    if (DeviceLayer::ConfigurationMgr().GetVendorId(value) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Discovery, "Vendor ID not known");
    }
    else
    {
        advertiseParameters.SetVendorId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::ConfigurationMgr().GetProductId(value) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Discovery, "Product ID not known");
    }
    else
    {
        advertiseParameters.SetProductId(chip::Optional<uint16_t>::Value(value));
    }

    if (DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(value) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Setup discriminator not known. Using a default.");
        value = 840;
    }
    advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>(value & 0xFF)).SetLongDiscrimininator(value);

    auto & mdnsAdvertiser = chip::Mdns::ServiceAdvertiser::Instance();

    ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));

    return mdnsAdvertiser.Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
}

/// (Re-)starts the minmdns server
void StartServer()
{
    CHIP_ERROR err = chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to start mDNS server: %s", chip::ErrorStr(err));
    }
}

} // namespace Mdns
} // namespace app
} // namespace chip
