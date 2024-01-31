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

#include "../core/MatterCommandTemplate-JNI.h"

#include <app-common/zap-generated/cluster-objects.h>

#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace matter {
namespace casting {
namespace clusters {

class ContentLauncherClusterLaunchURLCommand
    : public core::MatterCommandTemplateJNI<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type>
{
public:
    ContentLauncherClusterLaunchURLCommand(jobject jCommand) :
        core::MatterCommandTemplateJNI<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type>(jCommand)
    {}

    CHIP_ERROR GetCppRequestFromJava(jobject inRequest,
                                     chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type & outRequest);
    jobject GetJResponseFromCpp(const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType * response);
};

class MatterCommandManager
{
private:
    static const char * GetCommandClassName(jobject jCommandObject)
    {
        JNIEnv * env        = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        jclass commandClass = env->GetObjectClass(jCommandObject); // MatterCommand.class
        jclass clsClass     = env->GetObjectClass(commandClass);   // Class<Class>
        jmethodID mid       = env->GetMethodID(clsClass, "getName", "()Ljava/lang/String;");
        jstring jClassName  = (jstring) env->CallObjectMethod(commandClass, mid);
        return env->GetStringUTFChars(jClassName, nullptr);
    }

public:
    static jobject Invoke(jobject jCommand, jobject jRequest, jobject jTimedInvokeTimeoutMs)
    {
        const char * commandClassName = GetCommandClassName(jCommand);
        VerifyOrReturnValue(commandClassName != nullptr, nullptr,
                            ChipLogError(AppServer, "MatterCommandManager: Could not get commandClassName from jCommand"));

        if (strcmp(commandClassName, "com.matter.casting.clusters.MatterCommands$ContentLauncherClusterLaunchURLCommand") == 0)
        {
            auto * command = new clusters::ContentLauncherClusterLaunchURLCommand(jCommand);
            return command->Invoke(jRequest, jTimedInvokeTimeoutMs);
        }
        return nullptr;
    }
};

}; // namespace clusters
}; // namespace casting
}; // namespace matter
