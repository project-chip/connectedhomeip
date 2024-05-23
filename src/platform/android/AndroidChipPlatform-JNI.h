/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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
 *      Implementation of JNI bridge for CHIP Device Controller for Android apps
 *
 */

#pragma once

#include <jni.h>
#include <lib/core/CHIPError.h>

/**
    Load platform android jni functions
    To avoid load 2 dynamic libraries in one android app and
    each of them have their own matter librares,
    we build android platform as an static libray and
    link them in app (app/server or controller)
*/
CHIP_ERROR AndroidChipPlatformJNI_OnLoad(JavaVM * jvm, void * reserved);

void AndroidChipPlatformJNI_OnUnload(JavaVM * jvm, void * reserved);
