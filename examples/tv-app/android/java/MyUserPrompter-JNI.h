/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "CommissionerMain.h"
#include "lib/support/logging/CHIPLogging.h"
#include <jni.h>

class JNIMyUserPrompter : public UserPrompter
{
public:
    JNIMyUserPrompter(jobject prompter);
    void PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptForCommissionPincode(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override;
    void PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error) override;

private:
    jobject mJNIMyUserPrompterObject                 = nullptr;
    jmethodID mPromptForCommissionOKPermissionMethod = nullptr;
    jmethodID mPromptForCommissionPincodeMethod      = nullptr;
    jmethodID mPromptCommissioningSucceededMethod    = nullptr;
    jmethodID mPromptCommissioningFailedMethod       = nullptr;
};
