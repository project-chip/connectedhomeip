/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CastingPlayer.h"
#include "Types.h"

#include <controller/CHIPCommissionableNodeController.h>
#include <controller/DeviceDiscoveryDelegate.h>
#include <vector>

namespace matter {
namespace casting {
namespace core {

/**
 * @brief Represents CastingPlayerDiscovery state.
 *
 */
enum CastingPlayerDiscoveryState
{
    DISCOVERY_NOT_READY, // Default state, mClientDelegate isn't initialized
    DISCOVERY_READY,     // After SetDelegate and before StartDiscovery, mClientDelegate is initialized
    DISCOVERY_RUNNING,   // After StartDiscovery success
};

/**
 * @brief DiscoveryDelegate handles callbacks as CastingPlayers are discovered, updated, or lost
 * from the network.
 */
class DLL_EXPORT DiscoveryDelegate
{
public:
    virtual ~DiscoveryDelegate() {}
    virtual void HandleOnAdded(memory::Strong<CastingPlayer> player)    = 0;
    virtual void HandleOnUpdated(memory::Strong<CastingPlayer> players) = 0;
    // virtual void HandleOnRemoved(memory::Strong<CastingPlayer> players) = 0;
};

class CastingPlayerDiscovery;

/**
 * @brief DeviceDiscoveryDelegateImpl defines functionality for when callback
 * OnDiscoveredDevice is called.
 */
class DeviceDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate
{
private:
    DiscoveryDelegate * mClientDelegate = nullptr;

public:
    DeviceDiscoveryDelegateImpl() {}
    DeviceDiscoveryDelegateImpl(DiscoveryDelegate * delegate) { mClientDelegate = delegate; }

    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override;
};

/**
 * @brief CastingPlayerDiscovery is a singleton utility class for discovering CastingPlayers.
 */
class CastingPlayerDiscovery
{

private:
    std::vector<memory::Strong<CastingPlayer>> mCastingPlayers;
    DeviceDiscoveryDelegateImpl mDelegate;

    CastingPlayerDiscovery();
    static CastingPlayerDiscovery * _castingPlayerDiscovery;

    CastingPlayerDiscovery(CastingPlayerDiscovery & other) = delete;
    void operator=(const CastingPlayerDiscovery &)         = delete;

    chip::Controller::CommissionableNodeController mCommissionableNodeController;
    CastingPlayerDiscoveryState mState = DISCOVERY_NOT_READY;

public:
    static CastingPlayerDiscovery * GetInstance();

    /**
     * @brief Starts the discovery for CastingPlayers
     *
     * @param filterBydeviceType if passed as a non-zero value, CastingPlayerDiscovery will only callback on the DiscoveryDelegate
     * with CastingPlayers whose deviceType matches filterBydeviceType
     * @return CHIP_ERROR - CHIP_NO_ERROR if discovery for CastingPlayers started successfully, specific error code otherwise.
     */
    CHIP_ERROR StartDiscovery(uint32_t filterBydeviceType = 0);

    /**
     * @brief Stop the discovery for CastingPlayers
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if discovery for CastingPlayers stopped successfully, specific error code otherwise.
     */
    CHIP_ERROR StopDiscovery();

    void SetDelegate(DiscoveryDelegate * clientDelegate)
    {
        ChipLogProgress(Discovery, "CastingPlayerDiscovery::SetDelegate() called");
        if (clientDelegate == nullptr)
        {
            mState = DISCOVERY_NOT_READY;
        }
        else
        {
            mState = DISCOVERY_READY;
        }
        mDelegate = DeviceDiscoveryDelegateImpl(clientDelegate);
    }

    std::vector<memory::Strong<CastingPlayer>> GetCastingPlayers() { return mCastingPlayers; }

    friend class DeviceDiscoveryDelegateImpl;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
