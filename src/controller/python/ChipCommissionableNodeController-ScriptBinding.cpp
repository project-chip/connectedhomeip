/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      Implementation of the native methods expected by the Python
 *      version of CHIPCommissionableNodeController.
 *
 */

#include <controller/CHIPCommissionableNodeController.h>
#include <inet/IPAddress.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>

#include <type_traits>

using namespace chip;
using namespace chip::Controller;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {
ChipError::StorageType
pychip_CommissionableNodeController_NewController(chip::Controller::CommissionableNodeController ** outCommissionableNodeCtrl);
ChipError::StorageType
pychip_CommissionableNodeController_DeleteController(chip::Controller::CommissionableNodeController * commissionableNodeCtrl);

ChipError::StorageType
pychip_CommissionableNodeController_DiscoverCommissioners(chip::Controller::CommissionableNodeController * commissionableNodeCtrl);
void pychip_CommissionableNodeController_PrintDiscoveredCommissioners(
    chip::Controller::CommissionableNodeController * commissionableNodeCtrl);
}

ChipError::StorageType
pychip_CommissionableNodeController_NewController(chip::Controller::CommissionableNodeController ** outCommissionableNodeCtrl)
{
    *outCommissionableNodeCtrl = new (std::nothrow) chip::Controller::CommissionableNodeController();
    VerifyOrReturnError(*outCommissionableNodeCtrl != nullptr, CHIP_ERROR_NO_MEMORY.AsInteger());
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType
pychip_CommissionableNodeController_DeleteController(chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    if (commissionableNodeCtrl != nullptr)
    {
        delete commissionableNodeCtrl;
    }

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType
pychip_CommissionableNodeController_DiscoverCommissioners(chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    return commissionableNodeCtrl->DiscoverCommissioners().AsInteger();
}

void pychip_CommissionableNodeController_PrintDiscoveredCommissioners(
    chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; ++i)
    {
        const chip::Dnssd::DiscoveredNodeData * dnsSdInfo = commissionableNodeCtrl->GetDiscoveredCommissioner(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->commissionData.rotatingId, dnsSdInfo->commissionData.rotatingIdLen,
                                            rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissioner %d", i);
        ChipLogProgress(Discovery, "\tInstance name:\t\t%s", dnsSdInfo->commissionData.instanceName);
        ChipLogProgress(Discovery, "\tHost name:\t\t%s", dnsSdInfo->resolutionData.hostName);
        ChipLogProgress(Discovery, "\tPort:\t\t\t%u", dnsSdInfo->resolutionData.port);
        ChipLogProgress(Discovery, "\tLong discriminator:\t%u", dnsSdInfo->commissionData.longDiscriminator);
        ChipLogProgress(Discovery, "\tVendor ID:\t\t%u", dnsSdInfo->commissionData.vendorId);
        ChipLogProgress(Discovery, "\tProduct ID:\t\t%u", dnsSdInfo->commissionData.productId);
        ChipLogProgress(Discovery, "\tCommissioning Mode\t%u", dnsSdInfo->commissionData.commissioningMode);
        ChipLogProgress(Discovery, "\tDevice Type\t\t%u", dnsSdInfo->commissionData.deviceType);
        ChipLogProgress(Discovery, "\tDevice Name\t\t%s", dnsSdInfo->commissionData.deviceName);
        ChipLogProgress(Discovery, "\tRotating Id\t\t%s", rotatingId);
        ChipLogProgress(Discovery, "\tPairing Instruction\t%s", dnsSdInfo->commissionData.pairingInstruction);
        ChipLogProgress(Discovery, "\tPairing Hint\t\t%u", dnsSdInfo->commissionData.pairingHint);
        if (dnsSdInfo->resolutionData.GetMrpRetryIntervalIdle().HasValue())
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\t%u",
                            dnsSdInfo->resolutionData.GetMrpRetryIntervalIdle().Value().count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\tNot present");
        }
        if (dnsSdInfo->resolutionData.GetMrpRetryIntervalActive().HasValue())
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\t%u",
                            dnsSdInfo->resolutionData.GetMrpRetryIntervalActive().Value().count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\tNot present");
        }
        ChipLogProgress(Discovery, "\tSupports TCP\t\t%d", dnsSdInfo->resolutionData.supportsTcp);
        for (unsigned j = 0; j < dnsSdInfo->resolutionData.numIPs; ++j)
        {
            char buf[chip::Inet::IPAddress::kMaxStringLength];
            dnsSdInfo->resolutionData.ipAddress[j].ToString(buf);
            ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", j, buf);
        }
    }
}
