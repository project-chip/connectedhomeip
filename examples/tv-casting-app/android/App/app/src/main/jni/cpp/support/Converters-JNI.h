/*
 *
 *    Copyright (c) 2020-24 Project CHIP Authors
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

#include "core/BaseCluster.h"
#include "core/CastingPlayer.h"
#include "core/Command.h"
#include "core/Endpoint.h"

#include <lib/core/CHIPError.h>

#include <jni.h>

namespace matter {
namespace casting {
namespace support {

jobject convertMatterErrorFromCppToJava(CHIP_ERROR inErr);

/**
 * @brief Converts a native Endpoint into a MatterEndpoint jobject
 *
 * @return pointer to the Endpoint jobject if created successfully, nullptr otherwise.
 */
jobject convertEndpointFromCppToJava(matter::casting::memory::Strong<core::Endpoint> endpoint);

core::Endpoint * convertEndpointFromJavaToCpp(jobject jEndpointObject);

/**
 * @brief Convertes a native CastingPlayer into a MatterCastingPlayer jobject
 *
 * @param CastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 *
 * @return pointer to the CastingPlayer jobject if created successfully, nullptr otherwise.
 */
jobject convertCastingPlayerFromCppToJava(matter::casting::memory::Strong<core::CastingPlayer> player);

core::CastingPlayer * convertCastingPlayerFromJavaToCpp(jobject jCastingPlayerObject);

/**
 * @brief Converts a native Cluster into a MatterCluster jobject
 *
 * @return pointer to the Cluster jobject if created successfully, nullptr otherwise.
 */
jobject convertClusterFromCppToJava(matter::casting::memory::Strong<core::BaseCluster> cluster, const char * className);

core::BaseCluster * convertClusterFromJavaToCpp(jobject jClusterObject);

/**
 * @brief Converts a native Command into a MatterCommand jobject
 *
 * @return pointer to the Command jobject if created successfully, nullptr otherwise.
 */
jobject convertCommandFromCppToJava(void * command, const char * className);

void * convertCommandFromJavaToCpp(jobject jCommandObject);

jobject convertLongFromCppToJava(uint64_t responseData);

}; // namespace support
}; // namespace casting
}; // namespace matter
