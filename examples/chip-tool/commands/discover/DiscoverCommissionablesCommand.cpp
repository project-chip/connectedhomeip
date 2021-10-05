/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DiscoverCommissionablesCommand.h"
#include <lib/support/BytesToHex.h>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionablesCommand::Run()
{
    GetExecContext()->commissioner->RegisterDeviceDiscoveryDelegate(this);

    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kNone, (uint64_t) 0);
    return GetExecContext()->commissioner->DiscoverCommissionableNodes(filter);
}

void DiscoverCommissionablesCommand::OnDiscoveredDevice(const chip::Mdns::DiscoveredNodeData & nodeData)
{
    char rotatingId[chip::Mdns::kMaxRotatingIdLen * 2 + 1] = "";
    Encoding::BytesToUppercaseHexString(nodeData.rotatingId, nodeData.rotatingIdLen, rotatingId, sizeof(rotatingId));

    ChipLogProgress(Discovery, "Discovered Node: ");
    ChipLogProgress(Discovery, "\tHost name:\t\t%s", nodeData.hostName);
    ChipLogProgress(Discovery, "\tPort:\t\t\t%u", nodeData.port);
    ChipLogProgress(Discovery, "\tLong discriminator:\t%u", nodeData.longDiscriminator);
    ChipLogProgress(Discovery, "\tVendor ID:\t\t%u", nodeData.vendorId);
    ChipLogProgress(Discovery, "\tProduct ID:\t\t%u", nodeData.productId);
    ChipLogProgress(Discovery, "\tCommissioning Mode\t%u", nodeData.commissioningMode);
    ChipLogProgress(Discovery, "\tDevice Type\t\t%u", nodeData.deviceType);
    ChipLogProgress(Discovery, "\tDevice Name\t\t%s", nodeData.deviceName);
    ChipLogProgress(Discovery, "\tRotating Id\t\t%s", rotatingId);
    ChipLogProgress(Discovery, "\tPairing Instruction\t%s", nodeData.pairingInstruction);
    ChipLogProgress(Discovery, "\tPairing Hint\t\t0x%x", nodeData.pairingHint);
    for (int i = 0; i < nodeData.numIPs; i++)
    {
        char buf[chip::Inet::kMaxIPAddressStringLength];
        nodeData.ipAddress[i].ToString(buf);

        ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", i, buf);
    }
}
