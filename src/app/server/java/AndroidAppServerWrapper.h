/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <jni.h>
#include <lib/core/CHIPError.h>

CHIP_ERROR ChipAndroidAppInit(void);

void ChipAndroidAppShutdown(void);

jint AndroidAppServerJNI_OnLoad(JavaVM * jvm, void * reserved);

void AndroidAppServerJNI_OnUnload(JavaVM * jvm, void * reserved);
