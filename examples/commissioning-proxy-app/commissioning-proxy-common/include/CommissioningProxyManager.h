/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *    Copyright 2022-2024 Morse Micro
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

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include <lib/core/CHIPError.h>

// The CommissioningProxyManager class primarily represents the state of the commissioning Proxy
class CommissioningProxyManager
{
public:
    enum Action_t
    {
        CONNECT_ACTION = 0,
        DISCONNECT_ACTION,
        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_CPDisconnected = 0,
        kState_CPConnected
    } State;

    CHIP_ERROR Init();
    bool InitiateAction(Action_t aAction);

    using CommissioningProxyCallback_fn = std::function<void(Action_t)>;

    void SetCallbacks(CommissioningProxyCallback_fn aActionInitiated_CB, CommissioningProxyCallback_fn aActionCompleted_CB);


    // IsCPConnected() is usefully locally for retrieving the logical
    // state of the Commissioning Proxy
    bool IsCPConnected(void);

    // SetCPState() is used to set the state of the CP
    void SetCPState(uint8_t level);
    
#if 0
    // RestoreMainLight() is used if we are not sure what state the physical
    // main light is in, for example, after the Identify function may have used it
    void RestoreMainLight(void);
    // SetIdentifyLight() is for use by the Identify function, generally to
    // ensure the indication is turned off at the end of an Identify sequence.
    void SetIdentifyLight(bool aOn);

    // ToggleIdentifyLight() is for use by the Identify function to flash the
    // indication.
    void ToggleIdentifyLight(void);
    void ToggleIdentifyLight(uint32_t color);
#endif

private:
    friend CommissioningProxyManager & CommissioningProxyMgr(void);

    // MainLightState is the on/off state of the main light
    State_t mMainCommissioningProxyState;

    // StatusLightState is the on/off state of the secondary visible indicator
    State_t mStatusLightState;
    // StatusLightColor is the preferred color of the secondary visible indicator when on
    uint32_t mStatusLightColor;

    // The Identify indicator is one possible user of the secondary visible
    // indicator and is handled separately, as there may be other users to
    // combine together in future.
    // SecondaryLightState is the on/off state of the Identify indicator
    State_t mIdentifyLightState;
    // SecondaryLightColor is the preferred color of the Identify indicator when on
    uint32_t mIdentifyLightColor;

    CommissioningProxyCallback_fn mActionInitiated_CB;
    CommissioningProxyCallback_fn mActionCompleted_CB;
#if 0
    // SetMainLight() has a legacy bool interface and a later color interface
    // to support simple color selection when available externally
    void SetMainLight(bool aOn);
    void SetMainLight(uint32_t color);
    void ToggleMainLight(void);

    // SetStatusLight() has a legacy bool interface and a later color interface
    // to support simple color selection when available externally
    void SetStatusLight(bool aOn);
    void SetStatusLight(uint32_t color);
    void ToggleStatusLight(void);

    // SetIdentifyLight() has a color interface to support simple color
    // selection when available externally
    void SetIdentifyLight(uint32_t color);
#endif
    // There is a single static instance of the CommissioningProxyManager since the
    // physical light hardware is static as well
    static CommissioningProxyManager sCommissioningProxy;
};

inline CommissioningProxyManager & CommissioningProxyMgr(void)
{
    return CommissioningProxyManager::sCommissioningProxy;
}
