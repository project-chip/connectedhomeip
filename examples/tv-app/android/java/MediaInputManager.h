/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
