/*
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
