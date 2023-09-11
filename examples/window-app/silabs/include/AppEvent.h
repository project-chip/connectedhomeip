/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

#include <app/clusters/window-covering-server/window-covering-server.h>
#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters::WindowCovering;

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Light,
        kEventType_Install,
        kEventType_ButtonDown,
        kEventType_ButtonUp,

        kEventType_None,
        kEventType_Reset,
        kEventType_ResetPressed,
        kEventType_ResetWarning,
        kEventType_ResetCanceled,
        // Button events
        kEventType_UpPressed,
        kEventType_UpReleased,
        kEventType_DownPressed,
        kEventType_DownReleased,
        // Cover events
        kEventType_CoverChange,
        kEventType_CoverTypeChange,
        kEventType_TiltModeChange,

        // Cover Attribute update events
        kEventType_AttributeChange,

        // Provisioning events
        kEventType_ProvisionedStateChanged,
        kEventType_ConnectivityStateChanged,
        kEventType_BLEConnectionsChanged,
        kEventType_WinkOff,
        kEventType_WinkOn,
    };

    uint16_t Type;
    chip::EndpointId mEndpoint = 0;
    chip::AttributeId mAttributeId;

    union
    {
        struct
        {
            uint8_t Action;
        } ButtonEvent;
        struct
        {
            void * Context;
        } TimerEvent;

        struct
        {
            void * Context;
        } WindowEvent;
    };

    EventHandler Handler;
};
