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

namespace chip {
namespace Mdns {

/**
 * Initialize DNS-SD implementation for Android with an object of a class
 * that implements chip.devicecontroller.ServiceResolver interface.
 */
void InitializeWithObject(jobject resolverObject);

/**
 * Pass results of the service resolution to the CHIP stack.
 */
void HandleResolve(jstring instanceName, jstring serviceType, jstring address, jint port, jlong callbackHandle,
                   jlong contextHandle);

} // namespace Mdns
} // namespace chip
