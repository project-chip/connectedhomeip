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

#include <string>

#include <controller/CHIPDeviceController.h>
#include <controller/python/chip/native/PyChipError.h>
#include <json/json.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/dnssd/Resolver.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

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

        auto idleInterval = dnsSdInfo->GetMrpRetryIntervalIdle();
        if (idleInterval.has_value())
        {
            jsonVal["mrpRetryIntervalIdle"] = idleInterval->count();
        }

        auto activeInterval = dnsSdInfo->GetMrpRetryIntervalActive();
        if (activeInterval.has_value())
        {
            jsonVal["mrpRetryIntervalActive"] = activeInterval->count();
        }

        auto activeThreshold = dnsSdInfo->GetMrpRetryActiveThreshold();
        if (activeThreshold.has_value())
        {
            jsonVal["mrpRetryActiveThreshold"] = activeThreshold->count();
        }

        jsonVal["supportsTcpClient"] = dnsSdInfo->supportsTcpClient;
        jsonVal["supportsTcpServer"] = dnsSdInfo->supportsTcpServer;
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
            jsonVal["isICDOperatingAsLIT"] = *(dnsSdInfo->isICDOperatingAsLIT);
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

PyChipError pychip_CreateManualCode(uint16_t longDiscriminator, uint32_t passcode, char * manualCodeBuffer, size_t inBufSize,
                                    size_t * outBufSize)
{
    SetupPayload payload;
    SetupDiscriminator discriminator;
    discriminator.SetLongValue(longDiscriminator);
    payload.discriminator = discriminator;
    payload.setUpPINCode  = passcode;
    std::string setupManualCode;

    *outBufSize    = 0;
    CHIP_ERROR err = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(setupManualCode);
    if (err == CHIP_NO_ERROR)
    {
        MutableCharSpan span(manualCodeBuffer, inBufSize);
        // Plus 1 so we copy the null terminator
        CopyCharSpanToMutableCharSpan(CharSpan(setupManualCode.c_str(), setupManualCode.length() + 1), span);
        *outBufSize = span.size();
        if (*outBufSize == 0)
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
    }

    return ToPyChipError(err);
}
}
