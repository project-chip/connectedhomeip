/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <jni.h>
#include <lib/core/CHIPError.h>

CHIP_ERROR AndroidChipFabricProviderJNI_OnLoad(JavaVM * jvm, void * reserved);

void AndroidChipFabricProviderJNI_OnUnload(JavaVM * jvm, void * reserved);

CHIP_ERROR ReadFabricList(JNIEnv * env, jobject & self);
