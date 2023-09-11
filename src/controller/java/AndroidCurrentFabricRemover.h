/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
