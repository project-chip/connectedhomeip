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

class CheckInCompleteCallback
{
public:
    virtual ~CheckInCompleteCallback() {}

    /**
     * @brief Callback used to let the application know that a check-in message was received and validated.
     *
     * The callback will be executed in CHIP main loop. Implementations avoid blocking in this callback.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
     *                         node that sent the check-in message.
     */
    virtual void OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo);
};

class ChipToolCheckInDelegate : public chip::app::CheckInDelegate
{
public:
    virtual ~ChipToolCheckInDelegate() {}
    CHIP_ERROR Init(chip::app::ICDClientStorage * storage, chip::app::InteractionModelEngine * engine);
    void OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo) override;
    chip::app::RefreshKeySender * OnKeyRefreshNeeded(chip::app::ICDClientInfo & clientInfo,
                                                     chip::app::ICDClientStorage * clientStorage) override;
    void OnKeyRefreshDone(chip::app::RefreshKeySender * refreshKeySender, CHIP_ERROR error) override;

    /**
     * @brief Reigsters a callback when the check-in completes.
     *
     * The registeration will be processed inside CHIP main loop.
     *
     * @param[in] handler - A pointer to CheckInCompleteCallback to register.
     */
    void RegisterOnCheckInCompleteCallback(CheckInCompleteCallback * handler);

    /**
     * @brief Unreigsters a callback when the check-in completes.
     *
     * The unregisteration will be processed inside CHIP main loop.
     *
     * @param[in] handler - A pointer to CheckInCompleteCallback to unregister.
     */
    void UnregisterOnCheckInCompleteCallback(CheckInCompleteCallback * handler);

private:
    chip::app::ICDClientStorage * mpStorage        = nullptr;
    chip::app::InteractionModelEngine * mpImEngine = nullptr;

    std::set<CheckInCompleteCallback *> mCheckInCompleteCallbacks;
};
