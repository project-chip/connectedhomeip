/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Advertiser.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Dnssd {
namespace {

class NoneAdvertiser : public ServiceAdvertiser
{
public:
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * inetLayet) override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. DNS-SD init disabled.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    bool IsInitialized() override { return false; }

    void Shutdown() override {}

    CHIP_ERROR RemoveServices() override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. Removing services failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. Operational Advertisement failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. Commissioning Advertisement failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR FinalizeServiceUpdate() override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. Finalizing service update failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetCommissionableInstanceName(char * instanceName, size_t maxLength) const override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. DNS-SD GetCommissionableInstanceName not available.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR UpdateCommissionableInstanceName() override
    {
        ChipLogError(Discovery, "DNS-SD advertising not available. Can't update DNS-SD commissionable instance name.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

NoneAdvertiser gAdvertiser;

} // namespace

ServiceAdvertiser & ServiceAdvertiser::Instance()
{
    return gAdvertiser;
}

} // namespace Dnssd
} // namespace chip
