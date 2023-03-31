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

#include <controller/CurrentFabricRemover.h>
#include <jni.h>

namespace chip {
namespace Controller {

/**
 * AndroidCurrentFabricRemover class is a helper class that automatic cleanup in C++ class memory on Android Platform. Invoke the
 * java callback object after all the operations are complete.
 */
class AndroidCurrentFabricRemover : private CurrentFabricRemover
{
public:
    static CHIP_ERROR RemoveCurrentFabric(DeviceController * controller, NodeId remoteNodeId, jobject jcallback);

private:
    AndroidCurrentFabricRemover(DeviceController * controller, jobject javaCallbackObject);
    ~AndroidCurrentFabricRemover();

    static void OnRemoveCurrentFabric(void * context, NodeId remoteNodeId, CHIP_ERROR status);
    chip::Callback::Callback<OnCurrentFabricRemove> mOnRemoveCurrentFabricCallback;

    jobject mJavaCallback;
    jmethodID mOnSuccessMethod = nullptr;
    jmethodID mOnErrorMethod   = nullptr;
};

} // Namespace Controller
} // namespace chip
