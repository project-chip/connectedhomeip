/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DiscoverCommissionablesCommand.h"
#include <lib/support/BytesToHex.h>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionablesCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

void DiscoverCommissionablesCommand::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    nodeData.LogDetail();
}
