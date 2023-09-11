/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
