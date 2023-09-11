/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <app/server/AppDelegate.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

CHIP_ERROR ChipAndroidAppInit(AppDelegate * appDelegate = nullptr);

void ChipAndroidAppShutdown(void);

void ChipAndroidAppReset(void);

jint AndroidAppServerJNI_OnLoad(JavaVM * jvm, void * reserved);

void AndroidAppServerJNI_OnUnload(JavaVM * jvm, void * reserved);
