/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief The BLE operations a commissioning proxy needs that have no portable form.
 *
 * Everything else the BLE transport does — the BTP handshake, endpoint bookkeeping,
 * message relay — goes through chip::Ble, which every platform with a BLE stack
 * provides. Two things do not: switching the local BLE role, and driving a scan that
 * reports commissionable devices. Those live here so CommissioningProxyBleTransport
 * stays free of any platform header, and so a test can drive it against a fake.
 *
 * One implementation per platform. The application owns the instance and passes it to
 * the transport, which keeps a reference and never takes ownership.
 */
class CommissioningProxyBleAdapter
{
public:
    /**
     * Fired once per discovered commissionable device, on the Matter event loop, with
     * @p context passed back unchanged.
     *
     * Reported as scalars rather than as a cluster struct: the transport has to build
     * the ScanResultStruct from its own stored copy anyway (the emit path outlives this
     * call), so building one here too would duplicate that mapping. A BLE
     * advertisement carries no extended data and no Wi-Fi band, so there is nothing
     * else to report.
     *
     * @p address is the 6-byte BD_ADDR and is only valid for the duration of the call.
     */
    using DiscoveryCallback = void (*)(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                       uint16_t productId);

    virtual ~CommissioningProxyBleAdapter() = default;

    /**
     * Switch the local BLE role from peripheral to central.
     *
     * A commissioning proxy advertises as a peripheral so it can be commissioned onto
     * the fabric itself, then has to act as a central to reach the device it proxies
     * for. Called on the first ProxyConnectRequest over BLE, and again on later ones,
     * so it must succeed when the stack is already central.
     *
     *   - CHIP_NO_ERROR:   the stack is in central role.
     *   - CHIP_ERROR_BUSY: prior peripheral activity is still winding down; the
     *                      ProxyConnectRequest is answered BUSY and may be retried.
     *   - other error:     the switch failed; the request is answered FAILURE.
     */
    virtual CHIP_ERROR EnableCentralRole() = 0;

    /**
     * Start reporting commissionable devices through @p cb.
     *
     * BLE has a single scanner, shared by the foreground scan, the background scan and
     * connect, so this reports CHIP_ERROR_BUSY when the scanner is already held. The
     * transport arbitrates between its own users, so BUSY here means something outside
     * it holds the radio; the background-scan registry treats that as "defer and retry".
     */
    virtual CHIP_ERROR StartScan(DiscoveryCallback cb, void * context) = 0;

    /**
     * Stop a scan started by StartScan and release the scanner. A no-op when no scan is
     * running, so teardown paths can call it unconditionally.
     */
    virtual void StopScan() = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
