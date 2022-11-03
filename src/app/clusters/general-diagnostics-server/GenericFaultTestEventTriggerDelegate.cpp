/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
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
        ReturnErrorOnFailure(hwFaultsPrevious.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnErrorOnFailure(hwFaultsPrevious.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));

        ReturnErrorOnFailure(hwFaultsCurrent.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnErrorOnFailure(hwFaultsCurrent.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_SENSOR));
        ReturnErrorOnFailure(hwFaultsCurrent.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));
        ReturnErrorOnFailure(hwFaultsCurrent.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_USER_INTERFACE_FAULT));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnHardwareFaultsDetect(hwFaultsPrevious, hwFaultsCurrent);

        // Radio faults injection
        GeneralFaults<kMaxRadioFaults> radioFaultsPrevious;
        GeneralFaults<kMaxRadioFaults> radioFaultsCurrent;

        ReturnErrorOnFailure(radioFaultsPrevious.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnErrorOnFailure(radioFaultsPrevious.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));

        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_TYPE_CELLULAR_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));
        ReturnErrorOnFailure(radioFaultsCurrent.add(EMBER_ZCL_RADIO_FAULT_TYPE_NFC_FAULT));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnRadioFaultsDetect(radioFaultsPrevious, radioFaultsCurrent);

        GeneralFaults<kMaxNetworkFaults> networkFaultsPrevious;
        GeneralFaults<kMaxNetworkFaults> networkFaultsCurrent;

        // Network faults injections
        ReturnErrorOnFailure(networkFaultsPrevious.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnErrorOnFailure(networkFaultsPrevious.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));

        ReturnErrorOnFailure(networkFaultsCurrent.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnErrorOnFailure(networkFaultsCurrent.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));
        ReturnErrorOnFailure(networkFaultsCurrent.add(EMBER_ZCL_NETWORK_FAULT_TYPE_CONNECTION_FAILED));

        app::Clusters::GeneralDiagnosticsServer::Instance().OnNetworkFaultsDetect(networkFaultsPrevious, networkFaultsCurrent);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
