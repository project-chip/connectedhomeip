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

/**
 *    @file
 *      Implementation of ICD Client API for Android Platform
 *
 */

#pragma once

#include <app/icd/client/DefaultICDClientStorage.h>
#include <lib/support/JniReferences.h>

jobject getICDClientInfo(JNIEnv * env, const char * icdClientInfoSign, jint jFabricIndex);

CHIP_ERROR StoreICDEntryWithKey(JNIEnv * env, jint jFabricIndex, jobject jicdClientInfo, jbyteArray jKey);

CHIP_ERROR RemoveICDEntryWithKey(JNIEnv * env, jint jFabricIndex, jobject jicdClientInfo);

CHIP_ERROR ClearICDClientInfo(JNIEnv * env, jint jFabricIndex, jlong jNodeId);

chip::app::DefaultICDClientStorage * getICDClientStorage();
