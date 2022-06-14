/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
