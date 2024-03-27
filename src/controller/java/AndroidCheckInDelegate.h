/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/ICDClientStorage.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {
namespace app {

class InteractionModelEngine;

/// Callbacks for check in protocol
class AndroidCheckInDelegate : public CheckInDelegate
{
public:
    virtual ~AndroidCheckInDelegate() {}
    CHIP_ERROR Init(ICDClientStorage * storage, InteractionModelEngine * engine);
    void OnCheckInComplete(const ICDClientInfo & clientInfo) override;
    RefreshKeySender * OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage) override;
    void OnKeyRefreshDone(RefreshKeySender * refreshKeySender, CHIP_ERROR error) override;

    CHIP_ERROR SetDelegate(jobject checkInDelegateObj);

private:
    ICDClientStorage * mpStorage        = nullptr;
    InteractionModelEngine * mpImEngine = nullptr;

    JniGlobalReference mCheckInDelegate;
};

} // namespace app
} // namespace chip
