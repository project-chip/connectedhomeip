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
#include <commands/common/RemoteDataModelLogger.h>
#include <lib/support/BytesToHex.h>

using namespace ::chip;

void DiscoverCommissionablesCommandBase::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    nodeData.LogDetail();
    LogErrorOnFailure(RemoteDataModelLogger::LogDiscoveredNodeData(nodeData));
    SetCommandExitStatus(CHIP_NO_ERROR);
}

CHIP_ERROR DiscoverCommissionablesCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByShortDiscriminatorCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kShortDiscriminator, mDiscriminator);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByLongDiscriminatorCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kLongDiscriminator, mDiscriminator);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByCommissioningModeCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioningMode);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByVendorIdCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, mVendorId);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByDeviceTypeCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, mDeviceType);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}
