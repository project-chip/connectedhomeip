/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file defines the API for application specific callbacks.
 */

#pragma once

class AppDelegate
{
public:
    virtual ~AppDelegate() {}
    virtual void OnCommissioningSessionStarted() {}
    virtual void OnCommissioningSessionStopped() {}

    /*
     * This is called anytime a basic or enhanced commissioning window is opened.
     *
     * The type of the window can be retrieved by calling
     * CommissioningWindowManager::CommissioningWindowStatus()
     */
    virtual void OnCommissioningWindowOpened() {}
    virtual void OnCommissioningWindowClosed() {}
};
