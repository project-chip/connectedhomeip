/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of BleApplicationDelegate for Darwin platforms.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <ble/BleConfig.h>
#include <platform/Darwin/BleApplicationDelegate.h>

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        void BleApplicationDelegateImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj) {}
    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip
