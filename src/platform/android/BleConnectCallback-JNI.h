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

CHIP_ERROR BleConnectCallbackJNI_OnLoad(JavaVM * jvm, void * reserved);

void BleConnectCallbackJNI_OnUnload(JavaVM * jvm, void * reserved);
