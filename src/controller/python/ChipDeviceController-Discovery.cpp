/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      version of Chip Device Manager.
 *
 */

#include <controller/CHIPDeviceController.h>
#include <controller/python/chip/native/PyChipError.h>
#include <json/json.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/dnssd/Resolver.h>

using namespace chip;

typedef void (*IterateDiscoveredCommissionableNodesFunct)(const char * deviceInfoJson, size_t deviceInfoLen);

extern "C" {

bool pychip_DeviceController_HasDiscoveredCommissionableNode(Controller::DeviceCommissioner * devCtrl)
{
    for (int i = 0; i < devCtrl->GetMaxCommissionableNodesSupported(); ++i)
    {
        const Dnssd::CommissionNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        return true;
    }
    return false;
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodes(Controller::DeviceCommissioner * devCtrl, const uint8_t filterType,
                                                                const char * filterParam)
{
    Dnssd::DiscoveryFilter filter(static_cast<Dnssd::DiscoveryFilterType>(filterType));
    switch (static_cast<Dnssd::DiscoveryFilterType>(filterType))
    {
    case Dnssd::DiscoveryFilterType::kNone:
        break;
    case Dnssd::DiscoveryFilterType::kShortDiscriminator:
    case Dnssd::DiscoveryFilterType::kLongDiscriminator:
    case Dnssd::DiscoveryFilterType::kCompressedFabricId:
    case Dnssd::DiscoveryFilterType::kVendorId:
    case Dnssd::DiscoveryFilterType::kDeviceType: {
        // For any numerical filter, convert the string to a filter value
        errno                               = 0;
        unsigned long long int numericalArg = strtoull(filterParam, nullptr, 0);
        if ((numericalArg == ULLONG_MAX) && (errno == ERANGE))
        {
            return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
        }
        filter.code = static_cast<uint64_t>(numericalArg);
        break;
    }
    case Dnssd::DiscoveryFilterType::kCommissioningMode:
        break;
    case Dnssd::DiscoveryFilterType::kCommissioner:
        filter.code = 1;
        break;
    case Dnssd::DiscoveryFilterType::kInstanceName:
        filter.code         = 0;
        filter.instanceName = filterParam;
        break;
    default:
        return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
    }

    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_StopCommissionableDiscovery(Controller::DeviceCommissioner * devCtrl)
{
    return ToPyChipError(devCtrl->StopCommissionableDiscovery());
}

void pychip_DeviceController_IterateDiscoveredCommissionableNodes(Controller::DeviceCommissioner * devCtrl,
                                                                  IterateDiscoveredCommissionableNodesFunct cb)
{
    VerifyOrReturn(cb != nullptr);

    for (int i = 0; i < devCtrl->GetMaxCommissionableNodesSupported(); ++i)
    {
        const Dnssd::CommissionNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }

        Json::Value jsonVal;

        char rotatingId[Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->rotatingId, dnsSdInfo->rotatingIdLen, rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissionable Node %d", i);
        jsonVal["instanceName"]       = dnsSdInfo->instanceName;
        jsonVal["hostName"]           = dnsSdInfo->hostName;
        jsonVal["port"]               = dnsSdInfo->port;
        jsonVal["longDiscriminator"]  = dnsSdInfo->longDiscriminator;
        jsonVal["vendorId"]           = dnsSdInfo->vendorId;
        jsonVal["productId"]          = dnsSdInfo->productId;
        jsonVal["commissioningMode"]  = dnsSdInfo->commissioningMode;
        jsonVal["deviceType"]         = dnsSdInfo->deviceType;
        jsonVal["deviceName"]         = dnsSdInfo->deviceName;
        jsonVal["pairingInstruction"] = dnsSdInfo->pairingInstruction;
        jsonVal["pairingHint"]        = dnsSdInfo->pairingHint;
        if (dnsSdInfo->GetMrpRetryIntervalIdle().has_value())
        {
            jsonVal["mrpRetryIntervalIdle"] = dnsSdInfo->GetMrpRetryIntervalIdle()->count();
        }
        if (dnsSdInfo->GetMrpRetryIntervalActive().has_value())
        {
            jsonVal["mrpRetryIntervalActive"] = dnsSdInfo->GetMrpRetryIntervalActive()->count();
        }
        if (dnsSdInfo->GetMrpRetryActiveThreshold().has_value())
        {
            jsonVal["mrpRetryActiveThreshold"] = dnsSdInfo->GetMrpRetryActiveThreshold()->count();
        }
        jsonVal["supportsTcp"] = dnsSdInfo->supportsTcp;
        {
            Json::Value addresses;
            for (unsigned j = 0; j < dnsSdInfo->numIPs; ++j)
            {
                char buf[Inet::IPAddress::kMaxStringLength];
                dnsSdInfo->ipAddress[j].ToString(buf);
                addresses[j] = buf;
            }
            jsonVal["addresses"] = addresses;
        }
        if (dnsSdInfo->isICDOperatingAsLIT.has_value())
        {
            jsonVal["isICDOperatingAsLIT"] = *dnsSdInfo->isICDOperatingAsLIT;
        }
        if (dnsSdInfo->rotatingIdLen > 0)
        {
            jsonVal["rotatingId"] = rotatingId;
        }

        {
            auto str = jsonVal.toStyledString();
            cb(str.c_str(), str.size());
        }
    }
}

void pychip_DeviceController_PrintDiscoveredDevices(Controller::DeviceCommissioner * devCtrl)
{
    for (int i = 0; i < devCtrl->GetMaxCommissionableNodesSupported(); ++i)
    {
        const Dnssd::CommissionNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        char rotatingId[Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->rotatingId, dnsSdInfo->rotatingIdLen, rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissionable Node %d", i);
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
        if (dnsSdInfo->GetMrpRetryIntervalIdle().has_value())
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\t%u", dnsSdInfo->GetMrpRetryIntervalIdle()->count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\tNot present");
        }
        if (dnsSdInfo->GetMrpRetryIntervalActive().has_value())
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\t%u", dnsSdInfo->GetMrpRetryIntervalActive()->count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\tNot present");
        }
        ChipLogProgress(Discovery, "\tSupports TCP\t\t%d", dnsSdInfo->supportsTcp);
        if (dnsSdInfo->isICDOperatingAsLIT.has_value())
        {
            ChipLogProgress(Discovery, "\tICD is operating as a\t%s", *dnsSdInfo->isICDOperatingAsLIT ? "LIT" : "SIT");
        }
        for (unsigned j = 0; j < dnsSdInfo->numIPs; ++j)
        {
            char buf[Inet::IPAddress::kMaxStringLength];
            dnsSdInfo->ipAddress[j].ToString(buf);
            ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", j, buf);
        }
    }
}

bool pychip_DeviceController_GetIPForDiscoveredDevice(Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len)
{
    const Dnssd::CommissionNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(idx);
    if (dnsSdInfo == nullptr)
    {
        return false;
    }
    // TODO(cecille): Select which one we actually want.
    if (dnsSdInfo->ipAddress[0].ToString(addrStr, len) == addrStr)
    {
        return true;
    }
    return false;
}
}
