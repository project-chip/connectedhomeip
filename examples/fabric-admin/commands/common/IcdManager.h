/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app/icd/client/DefaultCheckInDelegate.h>

/**
 * @brief Manages check-ins from ICD devices.
 *
 * Intended to be used as a thin CheckInDelegate. This allows a delegate register
 * themselves so they can be aware when ICD device checks-in allowing the
 * delegate to interact with the ICD device during the short window that it is
 * awake.
 */
class IcdManager : public chip::app::DefaultCheckInDelegate
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate()                                                          = default;
        virtual void OnCheckInCompleted(const chip::app::ICDClientInfo & clientInfo) = 0;
    };

    static IcdManager & Instance();
    void OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo) override;

    // There is an assumption delegate assigned only happens once and that it lives
    // for the entirety of the lifetime of fabric admin.
    void SetDelegate(Delegate * delegate);

private:
    static IcdManager sInstance;
    Delegate * mDelegate = nullptr;
};
