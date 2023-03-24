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
     * CommissioningWindowManager::CommissioningWindowStatusForCluster(), but
     * being careful about how to handle the None status when a window is in
     * fact open.
     */
    virtual void OnCommissioningWindowOpened() {}
    virtual void OnCommissioningWindowClosed() {}
};
