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
#include <controller/python/chip/native/PyChipError.h>
#include <inet/IPAddress.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>

#include <type_traits>

using namespace chip;
using namespace chip::Controller;

extern "C" {
PyChipError
pychip_CommissionableNodeController_NewController(chip::Controller::CommissionableNodeController ** outCommissionableNodeCtrl);
PyChipError
pychip_CommissionableNodeController_DeleteController(chip::Controller::CommissionableNodeController * commissionableNodeCtrl);

PyChipError
pychip_CommissionableNodeController_DiscoverCommissioners(chip::Controller::CommissionableNodeController * commissionableNodeCtrl);
void pychip_CommissionableNodeController_PrintDiscoveredCommissioners(
    chip::Controller::CommissionableNodeController * commissionableNodeCtrl);
}

PyChipError
pychip_CommissionableNodeController_NewController(chip::Controller::CommissionableNodeController ** outCommissionableNodeCtrl)
{
    *outCommissionableNodeCtrl = new (std::nothrow) chip::Controller::CommissionableNodeController();
    VerifyOrReturnError(*outCommissionableNodeCtrl != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError
pychip_CommissionableNodeController_DeleteController(chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    if (commissionableNodeCtrl != nullptr)
    {
        delete commissionableNodeCtrl;
    }

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError
pychip_CommissionableNodeController_DiscoverCommissioners(chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    return ToPyChipError(commissionableNodeCtrl->DiscoverCommissioners());
}

void pychip_CommissionableNodeController_PrintDiscoveredCommissioners(
    chip::Controller::CommissionableNodeController * commissionableNodeCtrl)
{
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; ++i)
    {
        const chip::Dnssd::CommissionNodeData * dnsSdInfo = commissionableNodeCtrl->GetDiscoveredCommissioner(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->rotatingId, dnsSdInfo->rotatingIdLen, rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissioner %d", i);
        ChipLogProgress(Discovery, "\tInstance name:\t\t%s", dnsSdInfo->instanceName);
        ChipLogProgress(Discovery, "\tHost name:\t\t%s", dnsSdInfo->hostName);
        ChipLogProgress(Discovery, "\tPort:\t\t\t%u", dnsSdInfo->port);
        ChipLogProgress(Discovery, "\tLong discriminator:\t%u", dnsSdInfo->longDiscriminator);
        ChipLogProgress(Discovery, "\tVendor ID:\t\t%u", dnsSdInfo->vendorId);
        ChipLogProgress(Discovery, "\tProduct ID:\t\t%u", dnsSdInfo->productId);
        ChipLogProgress(Discovery, "\tCommissioning Mode\t%u", dnsSdInfo->commissioningMode);
        ChipLogProgress(Discovery, "\tDevice Type\t\t%u", dnsSdInfo->deviceType);
        ChipLogProgress(Discovery, "\tDevice Name\t\t%s", dnsSdInfo->deviceName);
        ChipLogProgress(Discovery, "\tRotating Id\t\t%s", rotatingId);
        ChipLogProgress(Discovery, "\tPairing Instruction\t%s", dnsSdInfo->pairingInstruction);
        ChipLogProgress(Discovery, "\tPairing Hint\t\t%u", dnsSdInfo->pairingHint);

        auto idleInterval = dnsSdInfo->GetMrpRetryIntervalIdle();
        if (idleInterval.has_value())
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\t%u", idleInterval->count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\tNot present");
        }

        auto activeInterval = dnsSdInfo->GetMrpRetryIntervalIdle();
        if (activeInterval.has_value())
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\t%u", activeInterval->count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\tNot present");
        }

        ChipLogProgress(Discovery, "\tSupports TCP Client\t\t%d", dnsSdInfo->supportsTcpClient);
        ChipLogProgress(Discovery, "\tSupports TCP Server\t\t%d", dnsSdInfo->supportsTcpServer);

        if (dnsSdInfo->isICDOperatingAsLIT.has_value())
        {
            ChipLogProgress(Discovery, "\tICD is operating as a\t%s", *(dnsSdInfo->isICDOperatingAsLIT) ? "LIT" : "SIT");
        }

        for (unsigned j = 0; j < dnsSdInfo->numIPs; ++j)
        {
            char buf[chip::Inet::IPAddress::kMaxStringLength];
            dnsSdInfo->ipAddress[j].ToString(buf);
            ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", j, buf);
        }
    }
}
