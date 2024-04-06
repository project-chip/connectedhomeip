/*
 *   Copyright (c) 2020-2024 Project CHIP Authors
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

#include "MatterEndpoint-JNI.h"

#include "../JNIDACProvider.h"
#include "../support/Converters-JNI.h"
#include "../support/MatterCallback-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "clusters/Clusters.h"           // from tv-casting-common
#include "core/CastingApp.h"             // from tv-casting-common
#include "core/CastingPlayer.h"          // from tv-casting-common
#include "core/CastingPlayerDiscovery.h" // from tv-casting-common
#include "core/Endpoint.h"               // from tv-casting-common

#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_MatterEndpoint_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

MatterEndpointJNI MatterEndpointJNI::sInstance;

JNI_METHOD(jint, getId)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterEndpoint-JNI::getId() called");
    Endpoint * endpoint = support::convertEndpointFromJavaToCpp(thiz);
    VerifyOrReturnValue(endpoint != nullptr, -1, ChipLogError(AppServer, "MatterEndpoint-JNI::getId() endpoint == nullptr"));
    return endpoint->GetId();
}

JNI_METHOD(jint, getProductId)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterEndpoint-JNI::getProductId() called");
    Endpoint * endpoint = support::convertEndpointFromJavaToCpp(thiz);
    VerifyOrReturnValue(endpoint != nullptr, -1, ChipLogError(AppServer, "MatterEndpoint-JNI::getProductId() endpoint == nullptr"));
    return endpoint->GetProductId();
}

JNI_METHOD(jint, getVendorId)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterEndpoint-JNI::getVendorId() called");
    Endpoint * endpoint = support::convertEndpointFromJavaToCpp(thiz);
    VerifyOrReturnValue(endpoint != nullptr, -1, ChipLogError(AppServer, "MatterEndpoint-JNI::getVendorId() endpoint == nullptr"));
    return endpoint->GetVendorId();
}

JNI_METHOD(jobject, getCastingPlayer)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterEndpoint-JNI::getCastingPlayer() called");
    Endpoint * endpoint = support::convertEndpointFromJavaToCpp(thiz);
    VerifyOrReturnValue(endpoint != nullptr, nullptr,
                        ChipLogError(AppServer, "MatterEndpoint-JNI::getCastingPlayer() endpoint == nullptr"));
    return support::convertCastingPlayerFromCppToJava(std::shared_ptr<CastingPlayer>(endpoint->GetCastingPlayer()));
}

}; // namespace core
}; // namespace casting
}; // namespace matter
