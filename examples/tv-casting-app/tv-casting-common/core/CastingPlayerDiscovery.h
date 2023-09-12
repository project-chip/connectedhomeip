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

#include "Types.h"
#include "CastingPlayer.h"

#include <vector>
#include <controller/CHIPCommissionableNodeController.h>
#include <controller/DeviceDiscoveryDelegate.h>

namespace matter {
namespace casting {
namespace core {

/**
 * @brief Represents CastingPlayerDiscovery state.
 *
 */
enum CastingPlayerDiscoveryState
{
    DISCOVERY_NOT_READY,  // Default state, mClientDelegate isn't initialized
    DISCOVERY_READY,      // After SetDelegate and before StartDiscovery, mClientDelegate is initialized
    DISCOVERY_RUNNING,    // After StartDiscovery success
};

class DLL_EXPORT DiscoveryDelegate
{
public:    
    virtual ~DiscoveryDelegate() {}
    virtual void HandleOnAdded(Strong<CastingPlayer> player) = 0;
    virtual void HandleOnUpdated(Strong<CastingPlayer> players) = 0;
    // virtual void HandleOnRemoved(std::vector<Strong<CastingPlayer>> players);
    
};

class CastingPlayerDiscovery;

class DeviceDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate {
private:
    DiscoveryDelegate * mClientDelegate = nullptr;

public:
    DeviceDiscoveryDelegateImpl(){}
    DeviceDiscoveryDelegateImpl(DiscoveryDelegate * delegate){
        mClientDelegate = delegate;
    }

    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override;
};

class CastingPlayerDiscovery
{

private:
    std::vector<Strong<CastingPlayer>> mCastingPlayers;
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
     * @return CHIP_ERROR - CHIP_NO_ERROR if discovery for CastingPlayers started successfully, specific error code otherwise.
     */
    CHIP_ERROR StartDiscovery(uint64_t deviceType = 0);

    /**
     * @brief Stop the discovery for CastingPlayers
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if discovery for CastingPlayers stopped successfully, specific error code otherwise.
     */
    CHIP_ERROR StopDiscovery();

    void SetDelegate(DiscoveryDelegate * clientDelegate)
    {
        if(clientDelegate == nullptr){
            mState = DISCOVERY_NOT_READY;
        }
        else{
            mState = DISCOVERY_READY;
        }
        mDelegate = DeviceDiscoveryDelegateImpl(clientDelegate);
        
    }

    std::vector<Strong<CastingPlayer>> * GetCastingPlayers(){return &mCastingPlayers;}
};

}; // namespace core
}; // namespace casting
}; // namespace matter