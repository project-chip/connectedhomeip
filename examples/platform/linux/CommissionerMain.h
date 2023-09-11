/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <controller/CHIPDeviceController.h>
#include <controller/CommissionerDiscoveryController.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <transport/TransportMgr.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

using chip::Controller::DeviceCommissioner;
using chip::Transport::PeerAddress;

CHIP_ERROR CommissionerPairOnNetwork(uint32_t pincode, uint16_t disc, PeerAddress address);
CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index);

CHIP_ERROR InitCommissioner(uint16_t commissionerPort, uint16_t udcListenPort, chip::FabricId fabricId = chip::kUndefinedFabricId);
void ShutdownCommissioner();

DeviceCommissioner * GetDeviceCommissioner();
CommissionerDiscoveryController * GetCommissionerDiscoveryController();

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
