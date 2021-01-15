/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// Prevent multiple inclusion
#pragma once

// User options for plugin Binding Table Library
#define EMBER_BINDING_TABLE_SIZE 10

/**** Network Section ****/
#define EMBER_SUPPORTED_NETWORKS (1)

#define EMBER_APS_UNICAST_MESSAGE_COUNT 10

/**** Cluster endpoint counts ****/
#define EMBER_AF_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_TEMP_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT (1)

/**** Cluster Plugins ****/

// Use this macro to check if the server side of the Basic cluster is included
#define ZCL_USING_BASIC_CLUSTER_SERVER
#define EMBER_AF_PLUGIN_BASIC_SERVER
#define EMBER_AF_PLUGIN_BASIC

// Use this macro to check if the server side of the Temperature Measurement cluster is included
#define ZCL_USING_TEMP_MEASUREMENT_CLUSTER_SERVER
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
#define EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT
