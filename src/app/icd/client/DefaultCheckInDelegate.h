/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

namespace chip {
namespace app {

using namespace std;

/// Callbacks for check in protocol
class DefaultCheckInDelegate : public CheckInDelegate
{
public:
    virtual ~DefaultCheckInDelegate() {}
    CHIP_ERROR Init(ICDClientStorage * storage);
    void OnCheckInComplete(const ICDClientInfo & clientInfo) override;
    RefreshKeySender * OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage) override;
    void OnKeyRefreshDone(RefreshKeySender * refreshKeySender, CHIP_ERROR error) override;

private:
    ICDClientStorage * mpStorage = nullptr;
};

} // namespace app
} // namespace chip
