/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRDefines.h>

/**
 * This file defines functions to resolve Matter cluster and attribute IDs into
 * strings.
 */

/**
 * Resolve Matter cluster IDs into a descriptive string.
 *
 * For unknown IDs, a string '<Unknown clusterID %d>' will be returned.
 */
MTR_EXTERN MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) NSString * MTRClusterNameForID(MTRClusterIDType clusterID);

/**
 * Resolve Matter attribute IDs into a descriptive string.
 *
 * For unknown IDs, a string '<Unknown clusterID %d>' (if the cluster ID is not known)
 * or '<Unknown attributeID %d>' (if the cluster ID is known but the attribute ID is not known)
 * will be returned.
 */
MTR_EXTERN MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) NSString * MTRAttributeNameForID(MTRClusterIDType clusterID, MTRAttributeIDType attributeID);

/**
 * Resolve Matter request (client to server) command IDs into a descriptive string.
 *
 * For unknown IDs, a string '<Unknown clusterID %d>' (if the cluster ID is not known)
 * or '<Unknown commandID %d>' (if the cluster ID is known but the command ID is not known)
 * will be returned.
 */
MTR_EXTERN MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2)) NSString * MTRRequestCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID);

/**
 * Resolve Matter response (server to client) command IDs into a descriptive string.
 *
 * For unknown IDs, a string '<Unknown clusterID %d>' (if the cluster ID is not known)
 * or '<Unknown commandID %d>' (if the cluster ID is known but the command ID is not known)
 * will be returned.
 */
MTR_EXTERN MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2)) NSString * MTRResponseCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID);

/**
 * Resolve Matter event IDs into a descriptive string.
 *
 * For unknown IDs, a string '<Unknown clusterID %d>' (if the cluster ID is not known)
 * or '<Unknown eventID %d>' (if the cluster ID is known but the event ID is not known)
 * will be returned.
 */
MTR_EXTERN MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2)) NSString * MTREventNameForID(MTRClusterIDType clusterID, MTREventIDType eventID);
