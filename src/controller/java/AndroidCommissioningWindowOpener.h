/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <controller/CommissioningWindowOpener.h>
#include <jni.h>

namespace chip {
namespace Controller {

/**
 * A helper class that can be used by consumers that don't care about the callback from the
 * open-commissioning-window process and just want automatic cleanup of the CommissioningWindowOpener when done
 * with it.
 */
class AndroidCommissioningWindowOpener : private CommissioningWindowOpener
{
public:
    // Takes the same arguments as CommissioningWindowOpener::OpenBasicCommissioningWindow except without the
    // callback.
    static CHIP_ERROR OpenBasicCommissioningWindow(DeviceController * controller, NodeId deviceId, System::Clock::Seconds16 timeout,
                                                   jobject jcallback);
    // Takes the same arguments as CommissioningWindowOpener::OpenCommissioningWindow except without the
    // callback.
    static CHIP_ERROR OpenCommissioningWindow(DeviceController * controller, NodeId deviceId, System::Clock::Seconds16 timeout,
                                              uint32_t iteration, uint16_t discriminator, Optional<uint32_t> setupPIN,
                                              Optional<ByteSpan> salt, jobject jcallback, SetupPayload & payload,
                                              bool readVIDPIDAttributes = false);

private:
    AndroidCommissioningWindowOpener(DeviceController * controller, jobject javaCallbackObject);
    ~AndroidCommissioningWindowOpener();

    static void OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload);
    static void OnOpenBasicCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status);

    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenBasicCommissioningWindow> mOnOpenBasicCommissioningWindowCallback;

    jobject mJavaCallback;
    jmethodID mOnSuccessMethod = nullptr;
    jmethodID mOnErrorMethod   = nullptr;
};

} // Namespace Controller
} // namespace chip
