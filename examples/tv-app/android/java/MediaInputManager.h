/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

class MediaInputManager
{
public:
    void InitializeWithObjects(jobject managerObject);
    CHIP_ERROR GetInputList(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR GetCurrentInput(chip::app::AttributeValueEncoder & aEncoder);
    bool SelectInput(uint8_t index);
    bool ShowInputStatus();
    bool HideInputStatus();
    bool RenameInput(uint8_t index, std::string name);

private:
    friend MediaInputManager & MediaInputMgr();

    static MediaInputManager sInstance;
    jobject mMediaInputManagerObject = nullptr;
    jmethodID mGetInputListMethod    = nullptr;
    jmethodID mGetCurrentInputMethod = nullptr;
    jmethodID mSelectInputMethod     = nullptr;
    jmethodID mShowInputStatusMethod = nullptr;
    jmethodID mHideInputStatusMethod = nullptr;
    jmethodID mRenameInputMethod     = nullptr;
};

inline class MediaInputManager & MediaInputMgr()
{
    return MediaInputManager::sInstance;
}
