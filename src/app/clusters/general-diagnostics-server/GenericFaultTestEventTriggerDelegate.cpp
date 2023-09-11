/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "GenericFaultTestEventTriggerDelegate.h"

#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <lib/support/CodeUtils.h>

using namespace ::chip::DeviceLayer;

namespace chip {

bool GenericFaultTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR GenericFaultTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{

    if ((eventTrigger & ~kGenericFaultQueryFabricIndexMask) == kGenericFaultQueryTrigger)
    {
        // Fault injection
        GeneralFaults<kMaxHardwareFaults> hwFaultsPrevious;
        GeneralFaults<kMaxHardwareFaults> hwFaultsCurrent;
        using app::Clusters::GeneralDiagnostics::HardwareFaultEnum;
        ReturnErrorOnFailure(hwFaultsPrevious.add(to_underlying(HardwareFaultEnum::kRadio)));
        ReturnErrorOnFailure(hwFaultsPrevious.add(to_underlying(HardwareFaultEnum::kPowerSource)));

        ReturnErrorOnFailure(hwFaultsCurrent.add(to_underlying(HardwareFaultEnum::kRadio)));
        ReturnErrorOnFailure(hwFaultsCurrent.add(to_underlying(HardwareFaultEnum::kSensor)));
        ReturnErrorOnFailure(hwFaultsCurrent.add(to_underlying(HardwareFaultEnum::kPowerSource)));
        ReturnErrorOnFailure(hwFaultsCurrent.add(to_underlying(HardwareFaultEnum::kUserInterfaceFault)));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnHardwareFaultsDetect(hwFaultsPrevious, hwFaultsCurrent);

        // Radio faults injection
        GeneralFaults<kMaxRadioFaults> radioFaultsPrevious;
        GeneralFaults<kMaxRadioFaults> radioFaultsCurrent;

        ReturnErrorOnFailure(radioFaultsPrevious.add(EMBER_ZCL_RADIO_FAULT_ENUM_WI_FI_FAULT));
        ReturnErrorOnFailure(radioFaultsPrevious.add(EMBER_ZCL_RADIO_FAULT_ENUM_THREAD_FAULT));

        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_ENUM_WI_FI_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_ENUM_CELLULAR_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_ENUM_THREAD_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_ENUM_NFC_FAULT));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnRadioFaultsDetect(radioFaultsPrevious, radioFaultsCurrent);

        GeneralFaults<kMaxNetworkFaults> networkFaultsPrevious;
        GeneralFaults<kMaxNetworkFaults> networkFaultsCurrent;

        // Network faults injections
        using app::Clusters::GeneralDiagnostics::NetworkFaultEnum;
        ReturnErrorOnFailure(networkFaultsPrevious.add(to_underlying(NetworkFaultEnum::kHardwareFailure)));
        ReturnErrorOnFailure(networkFaultsPrevious.add(to_underlying(NetworkFaultEnum::kNetworkJammed)));

        ReturnErrorOnFailure(networkFaultsCurrent.add(to_underlying(NetworkFaultEnum::kHardwareFailure)));
        ReturnErrorOnFailure(networkFaultsCurrent.add(to_underlying(NetworkFaultEnum::kNetworkJammed)));
        ReturnErrorOnFailure(networkFaultsCurrent.add(to_underlying(NetworkFaultEnum::kConnectionFailed)));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnNetworkFaultsDetect(networkFaultsPrevious, networkFaultsCurrent);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
