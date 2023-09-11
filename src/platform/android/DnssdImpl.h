/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

namespace chip {
namespace Dnssd {

/**
 * Initialize DNS-SD implementation for Android with an object of a class
 * that implements chip.devicecontroller.mdns.ServiceResolver interface, and an object of a class that implements
 * chip.devicecontroller.mdns.ChipMdnsCallback interface.
 */
void InitializeWithObjects(jobject resolverObject, jobject browserObject, jobject chipMdnsCallbackObject);

/**
 * Pass results of the service resolution to the CHIP stack.
 */
void HandleResolve(jstring instanceName, jstring serviceType, jstring hostName, jstring address, jint port, jobject textEntries,
                   jlong callbackHandle, jlong contextHandle);

void HandleBrowse(jobjectArray instanceName, jstring serviceType, jlong callbackHandle, jlong contextHandle);

} // namespace Dnssd
} // namespace chip
