/*
 *
 *    Copyright (c) 2020-24 Project CHIP Authors
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

#include "../support/Converters-JNI.h"
#include "../support/MatterCallback-JNI.h"
#include "core/Endpoint.h" // from tv-casting-common

#include <app/DeviceProxy.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace matter {
namespace casting {
namespace core {

class MatterEndpointJNI
{
public:
    MatterEndpointJNI() :
        mGetDeviceProxySuccessHandler([](chip::DeviceProxy * device) -> jobject {
            return support::convertLongFromCppToJava(reinterpret_cast<jlong>(device));
        })
    {}
    support::MatterCallbackJNI<chip::DeviceProxy *> mGetDeviceProxySuccessHandler;
    support::MatterFailureCallbackJNI mGetDeviceProxyFailureHandler;

private:
    friend MatterEndpointJNI & MatterEndpointJNIMgr();
    static MatterEndpointJNI sInstance;
};

inline class MatterEndpointJNI & MatterEndpointJNIMgr()
{
    return MatterEndpointJNI::sInstance;
}
}; // namespace core
}; // namespace casting
}; // namespace matter
