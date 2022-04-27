/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Defines the Device Layer DeviceControlServer object.
 */

#pragma once

#include <lib/support/Span.h>
#include <platform/FailSafeContext.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

/**
 * Defines the Swtich Device Control Delegate class to notify platform events.
 */
class SwitchDeviceControlDelegate
{
public:
    virtual ~SwitchDeviceControlDelegate() {}

    /**
     * @brief
     *   Called when the latching switch is moved to a new position.
     */
    virtual void OnSwitchLatched(uint8_t newPosition) {}

    /**
     * @brief
     *   Called when the momentary switch starts to be pressed.
     */
    virtual void OnInitialPressed(uint8_t newPosition) {}

    /**
     * @brief
     *   Called when the momentary switch has been pressed for a "long" time.
     */
    virtual void OnLongPressed(uint8_t newPosition) {}

    /**
     * @brief
     *   Called when the momentary switch has been released.
     */
    virtual void OnShortReleased(uint8_t previousPosition) {}

    /**
     * @brief
     *   Called when the momentary switch has been released (after debouncing)
     *   and after having been pressed for a long time.
     */
    virtual void OnLongReleased(uint8_t previousPosition) {}

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, during that sequence.
     */
    virtual void OnMultiPressOngoing(uint8_t newPosition, uint8_t count) {}

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, after it has been detected that the sequence has ended.
     */
    virtual void OnMultiPressComplete(uint8_t newPosition, uint8_t count) {}
};

class DeviceControlServer final
{
public:
    // ===== Members for internal use by other Device Layer components.

    CHIP_ERROR CommissioningComplete(NodeId peerNodeId, FabricIndex accessingFabricIndex);
    CHIP_ERROR SetRegulatoryConfig(uint8_t location, const CharSpan & countryCode);
    CHIP_ERROR ConnectNetworkForOperational(ByteSpan networkID);

    void SetSwitchDelegate(SwitchDeviceControlDelegate * delegate) { mSwitchDelegate = delegate; }
    SwitchDeviceControlDelegate * GetSwitchDelegate() const { return mSwitchDelegate; }
    FailSafeContext & GetFailSafeContext() { return mFailSafeContext; }

    static DeviceControlServer & DeviceControlSvr();

private:
    // ===== Members for internal use by the following friends.
    static DeviceControlServer sInstance;
    FailSafeContext mFailSafeContext;
    SwitchDeviceControlDelegate * mSwitchDelegate = nullptr;

    // ===== Private members reserved for use by this class only.

    DeviceControlServer()  = default;
    ~DeviceControlServer() = default;

    // No copy, move or assignment.
    DeviceControlServer(const DeviceControlServer &)  = delete;
    DeviceControlServer(const DeviceControlServer &&) = delete;
    DeviceControlServer & operator=(const DeviceControlServer &) = delete;
};

} // namespace DeviceLayer
} // namespace chip
