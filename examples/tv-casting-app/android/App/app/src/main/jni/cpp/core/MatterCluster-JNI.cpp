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

#include "MatterCluster-JNI.h"

#include "../JNIDACProvider.h"
#include "../support/Converters-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "core/CastingApp.h" // from tv-casting-common

#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_MatterCluster_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

JNI_METHOD(jobject, getEndpoint)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCluster-JNI::getEndpoint() called");
    BaseCluster * cluster = support::convertClusterFromJavaToCpp(thiz);
    VerifyOrReturnValue(cluster != nullptr, 0, ChipLogError(AppServer, "MatterCluster-JNI::getEndpoint() cluster == nullptr"));
    return support::convertEndpointFromCppToJava(std::shared_ptr<Endpoint>(cluster->GetEndpoint().lock()));
}

JNI_METHOD(jobject, getCommand)
(JNIEnv * env, jobject thiz, jclass commandClass)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCluster-JNI::getCommand() called");

    jclass clsClass        = env->GetObjectClass(commandClass); // Class<Class>
    jmethodID mid          = env->GetMethodID(clsClass, "getName", "()Ljava/lang/String;");
    jstring jClassName     = (jstring) env->CallObjectMethod(commandClass, mid);
    const char * className = env->GetStringUTFChars(jClassName, nullptr);
    ChipLogProgress(AppServer, "MatterCluster-JNI::getCommand() className: %s", className);
    if (strcmp(className, "com.matter.casting.clusters.MatterCommands$ContentLauncherClusterLaunchURLCommand") == 0)
    {
        BaseCluster * cluster = support::convertClusterFromJavaToCpp(thiz);
        VerifyOrReturnValue(cluster != nullptr, 0, ChipLogError(AppServer, "MatterCluster-JNI::getCommand() cluster == nullptr"));
        void * command = cluster->GetCommand(chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Id);

        jobject commandJavaObject = support::convertCommandFromCppToJava(
            command, "com/matter/casting/clusters/MatterCommands$ContentLauncherClusterLaunchURLCommand");
        env->ReleaseStringUTFChars(jClassName, className);
        return commandJavaObject;
    }

    env->ReleaseStringUTFChars(jClassName, className);
    return nullptr;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
