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
#include <app/clusters/media-input-server/media-input-server.h>
#include <jni.h>

class MediaInputManager : public chip::app::Clusters::MediaInput::Delegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    CHIP_ERROR HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder) override;
    uint8_t HandleGetCurrentInput() override;
    bool HandleSelectInput(const uint8_t index) override;
    bool HandleShowInputStatus() override;
    bool HandleHideInputStatus() override;
    bool HandleRenameInput(const uint8_t index, const chip::CharSpan & name) override;

private:
    jobject mMediaInputManagerObject = nullptr;
    jmethodID mGetInputListMethod    = nullptr;
    jmethodID mGetCurrentInputMethod = nullptr;
    jmethodID mSelectInputMethod     = nullptr;
    jmethodID mShowInputStatusMethod = nullptr;
    jmethodID mHideInputStatusMethod = nullptr;
    jmethodID mRenameInputMethod     = nullptr;
};
