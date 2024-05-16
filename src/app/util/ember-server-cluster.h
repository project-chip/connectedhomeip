/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app/ServerCluster.h>

#include <app-common/zap-generated/callback.h>
#include <app/util/config.h>

/**
 * Defines static storage for the specified number of instances of the specified class
 * (which must be a sub-class of chip::app::ServerCluster) as well as the corresponding Ember
 * ServerInitCallback and ShutdownCallback functions.
 *
 * Note: This macro must be called from the global namespace.
 *
 * Example:
 * DECLARE_SERVER_CLUSTER(DISCO_BALL_CLUSTER, chip::app::Clusters::DiscoBall::Instance);
 */
#define DECLARE_SERVER_CLUSTER(NAME, CLASS)                                                                                        \
    chip::app::EmberClusterAdapter<CLASS, _DECLARE_CLUSTER_EXPAND(MATTER_DM_##NAME##_SERVER_ENDPOINT_COUNT)>                       \
        gClusterAdapter_##NAME;                                                                                                    \
    void _DECLARE_CLUSTER_CONCAT3(emberAf, MATTER_DM_##NAME##_LABEL, ClusterServerInitCallback)(chip::EndpointId endpoint)         \
    {                                                                                                                              \
        gClusterAdapter_##NAME.Init(endpoint);                                                                                     \
    }                                                                                                                              \
    void _DECLARE_CLUSTER_CONCAT3(Matter, MATTER_DM_##NAME##_LABEL, ClusterServerShutdownCallback)(chip::EndpointId endpoint)      \
    {                                                                                                                              \
        gClusterAdapter_##NAME.Shutdown(endpoint);                                                                                 \
    }

#define _DECLARE_CLUSTER_EXPAND(a) a
#define _DECLARE_CLUSTER_CONCAT3_(a, b, c) a##b##c
#define _DECLARE_CLUSTER_CONCAT3(a, b, c) _DECLARE_CLUSTER_CONCAT3_(a, b, c)
