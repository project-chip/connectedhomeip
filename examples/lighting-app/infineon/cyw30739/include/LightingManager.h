/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/CHIPError.h>

class LightingManager
{
public:
    enum Action_t
    {
        ON_ACTION = 0,
        OFF_ACTION,
        LEVEL_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_OffInitiated = 0,
        kState_OffCompleted,
        kState_OnInitiated,
        kState_OnCompleted,
    } State;

    enum Actor_t
    {
        ACTOR_ZCL_CMD = 0,
        ACTOR_APP_CMD,
        ACTOR_BUTTON,
    } Actor;

    CHIP_ERROR Init();
    bool IsLightOn();
    bool IsActionInProgress();
    bool InitiateAction(Actor_t aActor, Action_t aAction, uint8_t vallue);

    typedef void (*Callback_fn_initiated)(Actor_t, Action_t, uint8_t);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);
    void WriteClusterLevel(uint8_t value);

private:
    void WriteClusterState(uint8_t value);

    friend LightingManager & LightMgr(void);
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    static LightingManager sLight;
};

inline LightingManager & LightMgr(void)
{
    return LightingManager::sLight;
}
