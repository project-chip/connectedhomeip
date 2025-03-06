/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include "IcdManager.h"

IcdManager IcdManager::sInstance;

IcdManager & IcdManager::Instance()
{
    return sInstance;
}

void IcdManager::OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo)
{
    DefaultCheckInDelegate::OnCheckInComplete(clientInfo);
    if (mDelegate)
    {
        mDelegate->OnCheckInCompleted(clientInfo);
    }
}

void IcdManager::SetDelegate(Delegate * delegate)
{
    // To keep IcdManager simple, there is an assumption that there is only ever
    // one delegate set and it's lifetime is identical to IcdManager. In the
    // future this assumption can change should there be a need, but that will
    // require code changes to IcdManager. For now we will crash if someone tries
    // to call SetDelegate for a second time or if delegate is non-null.
    VerifyOrDie(delegate);
    VerifyOrDie(!mDelegate);
    mDelegate = delegate;
}
