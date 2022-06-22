/*
 *    Copyright (c) 2022 Project CHIP Authors
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
