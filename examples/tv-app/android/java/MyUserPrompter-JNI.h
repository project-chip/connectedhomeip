/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "CommissionerMain.h"
#include "lib/support/logging/CHIPLogging.h"
#include <jni.h>
#include <lib/support/JniReferences.h>

class JNIMyUserPrompter : public UserPrompter
{
public:
    // TODO
    JNIMyUserPrompter(jobject prompter);
    void PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptForCommissionPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName, uint16_t pairingHint,
                                     const char * pairingInstruction) override;
    void HidePromptsOnCancel(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    bool DisplaysPasscodeAndQRCode() override;
    void PromptWithCommissionerPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName, uint32_t passcode,
                                        uint16_t pairingHint, const char * pairingInstruction) override;
    void PromptCommissioningStarted(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error) override;

private:
    chip::JniGlobalReference mJNIMyUserPrompterObject;
    jmethodID mPromptForCommissionOKPermissionMethod = nullptr;
    jmethodID mPromptForCommissionPincodeMethod      = nullptr;
    jmethodID mHidePromptsOnCancelMethod             = nullptr;
    jmethodID mPromptWithCommissionerPasscodeMethod  = nullptr;
    jmethodID mPromptCommissioningStartedMethod      = nullptr;
    jmethodID mPromptCommissioningSucceededMethod    = nullptr;
    jmethodID mPromptCommissioningFailedMethod       = nullptr;
};
