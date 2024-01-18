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
#include <commands/common/DeviceScanner.h>
#include <commands/common/RemoteDataModelLogger.h>
#include <lib/support/BytesToHex.h>

using namespace ::chip;

void DiscoverCommissionablesCommandBase::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    nodeData.LogDetail();
    LogErrorOnFailure(RemoteDataModelLogger::LogDiscoveredNodeData(nodeData));

    if (mDiscoverOnce.ValueOr(true))
    {
        mCommissioner->RegisterDeviceDiscoveryDelegate(nullptr);
        auto err = mCommissioner->StopCommissionableDiscovery();
        SetCommandExitStatus(err);
    }
}

CHIP_ERROR DiscoverCommissionablesStartCommand::RunCommand()
{
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GetDeviceScanner().Start());

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
}

CHIP_ERROR DiscoverCommissionablesStopCommand::RunCommand()
{
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GetDeviceScanner().Stop());

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
}

CHIP_ERROR DiscoverCommissionablesListCommand::RunCommand()
{
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);
    GetDeviceScanner().Log();

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
}

CHIP_ERROR DiscoverCommissionablesCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByShortDiscriminatorCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kShortDiscriminator, mDiscriminator);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByLongDiscriminatorCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kLongDiscriminator, mDiscriminator);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByCommissioningModeCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioningMode);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByVendorIdCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, mVendorId);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoverCommissionableByDeviceTypeCommand::RunCommand()
{
    mCommissioner = &CurrentCommissioner();
    mCommissioner->RegisterDeviceDiscoveryDelegate(this);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, mDeviceType);
    return mCommissioner->DiscoverCommissionableNodes(filter);
}
