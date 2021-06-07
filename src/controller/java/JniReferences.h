/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <jni.h>

#include <core/CHIPError.h>

namespace {
JavaVM * sJvm = nullptr;
} // namespace

void SetJavaVm(JavaVM * jvm);
JNIEnv * GetEnvForCurrentThread();

CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls);
CHIP_ERROR FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature, jmethodID * methodId);
void CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument);
