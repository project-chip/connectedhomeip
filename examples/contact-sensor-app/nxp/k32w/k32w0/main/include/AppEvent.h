/*
 *
 * SPDX-FileCopyrightText: 2022 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

struct AppEvent;
typedef void (*EventHandler)(void *);

struct AppEvent
{
    enum AppEventTypes
    {
        kButton = 0,
        kTimer,
        kContact,
        kInstall,
#if defined(chip_with_low_power) && (chip_with_low_power == 1)
        kEventType_Lp,
#endif
        kOTAResume,
    };

    AppEventTypes Type;

    union
    {
        struct
        {
            uint8_t PinNo;
            uint8_t Action;
        } ButtonEvent;
        struct
        {
            void * Context;
        } TimerEvent;
        struct
        {
            uint8_t Action;
        } ContactEvent;
    };

    EventHandler Handler;

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    void * param;
#endif
};
