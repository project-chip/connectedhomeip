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

#include <set>

#include <app/InteractionModelEngine.h>
#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/ICDClientStorage.h>

class ChipToolCheckInDelegate : public chip::app::CheckInDelegate
{
public:
    virtual ~ChipToolCheckInDelegate() {}
    CHIP_ERROR Init(chip::app::ICDClientStorage * storage, chip::app::InteractionModelEngine * engine);
    void OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo) override;
    chip::app::RefreshKeySender * OnKeyRefreshNeeded(chip::app::ICDClientInfo & clientInfo,
                                                     chip::app::ICDClientStorage * clientStorage) override;
    void OnKeyRefreshDone(chip::app::RefreshKeySender * refreshKeySender, CHIP_ERROR error) override;

private:
    chip::app::ICDClientStorage * mpStorage        = nullptr;
    chip::app::InteractionModelEngine * mpImEngine = nullptr;
};
