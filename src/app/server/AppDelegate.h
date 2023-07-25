/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *   This file defines the API for application specific callbacks.
 */

#pragma once

#include <lib/core/CHIPError.h>

class AppDelegate
{
public:
    virtual ~AppDelegate() {}
    /**
     * This is called when the PBKDFParamRequest is received and indicates the start of the session establishment process
     */
    virtual void OnCommissioningSessionEstablishmentStarted() {}

    /**
     * This is called when the commissioning session has been established
     */
    virtual void OnCommissioningSessionStarted() {}

    /**
     * This is called when there is an error in establishing a commissioning session (such as, when an invalid passcode is provided)
     *
     * @param err CHIP_ERROR indicating the error that occurred during session establishment
     *
     * @return true if the commissioning window should be closed, false otherwise
     */
    virtual bool OnCommissioningSessionEstablishmentError(CHIP_ERROR err) { return false; }

    /**
     * This is called in addition to OnCommissioningSessionEstablishmentError i.e. when there is an error in establishing a
     * commissioning session AND the commissioning window is closed. The window may be closed because
     * OnCommissioningSessionEstablishmentError returned 'true', the commissioning attempts limit was reached or listening for PASE
     * failed.
     */
    virtual void OnCommissioningSessionStopped() {}

    /*
     * This is called anytime a basic or enhanced commissioning window is opened.
     *
     * The type of the window can be retrieved by calling
     * CommissioningWindowManager::CommissioningWindowStatusForCluster(), but
     * being careful about how to handle the None status when a window is in
     * fact open.
     */
    virtual void OnCommissioningWindowOpened() {}

    /*
     * This is called anytime a basic or enhanced commissioning window is closed.
     */
    virtual void OnCommissioningWindowClosed() {}
};
