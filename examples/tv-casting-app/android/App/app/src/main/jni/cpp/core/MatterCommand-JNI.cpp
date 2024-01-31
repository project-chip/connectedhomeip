/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "MatterCommand-JNI.h"
#include "../clusters/MatterCommandObjects-JNI.h"

#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_MatterCommand_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

JNI_METHOD(jobject, invoke)
(JNIEnv * env, jobject thiz, jobject jRequest, jobject jTimedInvokeTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCommand-JNI::invoke() called");
    return clusters::MatterCommandManager::Invoke(thiz, jRequest, jTimedInvokeTimeoutMs);
}

}; // namespace core
}; // namespace casting
}; // namespace matter
