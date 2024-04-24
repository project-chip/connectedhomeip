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

#include "AndroidCurrentFabricRemover.h"

#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {
namespace Controller {

AndroidCurrentFabricRemover::AndroidCurrentFabricRemover(DeviceController * controller, jobject jCallbackObject) :
    CurrentFabricRemover(controller), mOnRemoveCurrentFabricCallback(OnRemoveCurrentFabric, this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    if (mJavaCallback.Init(jCallbackObject) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Fail to init mJavaObjectRef");
        return;
    }
    jclass callbackClass = env->GetObjectClass(jCallbackObject);

    mOnSuccessMethod = env->GetMethodID(callbackClass, "onSuccess", "(J)V");
    if (mOnSuccessMethod == nullptr)
    {
        ChipLogError(Controller, "Failed to access callback 'onSuccess' method");
        env->ExceptionClear();
    }

    mOnErrorMethod = env->GetMethodID(callbackClass, "onError", "(IJ)V");
    if (mOnErrorMethod == nullptr)
    {
        ChipLogError(Controller, "Failed to access callback 'onError' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR AndroidCurrentFabricRemover::RemoveCurrentFabric(DeviceController * controller, NodeId remoteNodeId, jobject jcallback)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * remover = new AndroidCurrentFabricRemover(controller, jcallback);
    if (remover == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = remover->CurrentFabricRemover::RemoveCurrentFabric(remoteNodeId, &remover->mOnRemoveCurrentFabricCallback);
    if (err != CHIP_NO_ERROR)
    {
        delete remover;
        remover = nullptr;
    }
    // Else will clean up when the callback is called.
    return err;
}

void AndroidCurrentFabricRemover::OnRemoveCurrentFabric(void * context, NodeId remoteNodeId, CHIP_ERROR err)
{
    auto * self = static_cast<AndroidCurrentFabricRemover *>(context);

    if (self != nullptr && self->mJavaCallback.HasValidObjectRef())
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        if (err == CHIP_NO_ERROR)
        {
            if (self->mOnSuccessMethod != nullptr)
            {
                env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnSuccessMethod, static_cast<jlong>(remoteNodeId));
            }
        }
        else
        {
            if (self->mOnErrorMethod != nullptr)
            {
                env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnErrorMethod, static_cast<jint>(err.GetValue()),
                                    static_cast<jlong>(remoteNodeId));
            }
        }
    }

    delete self;
}

} // namespace Controller
} // namespace chip
