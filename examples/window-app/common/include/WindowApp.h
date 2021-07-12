/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "ExampleApp.h"
#include <app/common/gen/enums.h>

namespace example {

enum class WindowEvent
{
    None = 0,
    Reset,
    // Button events
    UpPressed,
    UpReleased,
    DownPressed,
    DownReleased,
    // Cover events
    CoverTypeChange,
    CoverTiltModeChange,
    CoverLiftUp,
    CoverLiftDown,
    CoverLiftChanged,
    CoverTiltUp,
    CoverTiltDown,
    CoverTiltChanged,
    CoverStop
};

class WindowApp : public App<WindowEvent>
{

public:
    struct StateFlags
    {
        bool upPressed               = false;
        bool downPressed             = false;
        bool longPress               = false;
        bool tiltMode                = false;
        bool resetWarning            = false;
        bool isThreadProvisioned     = false;
        bool isThreadEnabled         = false;
        bool haveBLEConnections      = false;
        bool haveServiceConnectivity = false;
    };

    static WindowApp & Instance();

    virtual ~WindowApp();

    virtual CHIP_ERROR Init();
    virtual void Finish();
    virtual CHIP_ERROR Run();
    virtual void PostEvent(const WindowEvent & event) = 0;

private:
    virtual void OnLiftUp();
    virtual void OnLiftDown();
    virtual void OnTiltUp();
    virtual void OnTiltDown();

protected:
    StateFlags mState;
    WindowEvent mLiftAction = WindowEvent::None;
    WindowEvent mTiltAction = WindowEvent::None;

    example::Timer * mLiftTimer      = nullptr;
    example::Timer * mTiltTimer      = nullptr;
    example::Timer * mResetTimer     = nullptr;
    example::Timer * mCoverTypeTimer = nullptr;

    virtual void ProcessEvents() = 0;
    virtual void DispatchEvent(const WindowEvent & event);
    virtual Timer * CreateTimer(const char * name, uint32_t timeoutInMs, example::Timer::Callback callback, void * context) = 0;
    virtual void DestroyTimer(Timer * timer)                                                                                = 0;

    virtual void OnMainLoop()                                  = 0;
    virtual void OnCoverTypeChanged()                          = 0;
    virtual void OnLiftChanged()                               = 0;
    virtual void OnTiltChanged()                               = 0;
    virtual void OnTiltModeChanged()                           = 0;
    virtual void OnProvisionedStateChanged(bool isProvisioned) = 0;
    virtual void OnConnectivityStateChanged(bool isConnected)  = 0;
    virtual void OnBLEConnectionsChanged(bool haveConnections) = 0;
    virtual void OnResetWarning()                              = 0;
    virtual void OnResetCanceled()                             = 0;
    virtual void OnReset()                                     = 0;

    static void OnLiftTimeout(example::Timer & timer);
    static void OnTiltTimeout(example::Timer & timer);
    static void OnCoverTypeTimeout(example::Timer & timer);
    static void OnResetTimeout(example::Timer & timer);

private:
    void GotoLift();
    void GotoTilt();
    void CoverTypeCycle();
};

} // namespace example
