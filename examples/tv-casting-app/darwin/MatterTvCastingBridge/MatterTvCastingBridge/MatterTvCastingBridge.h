/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import <Foundation/Foundation.h>

//! Project version number for MatterTvCastingBridge.
FOUNDATION_EXPORT double MatterTvCastingBridgeVersionNumber;

//! Project version string for MatterTvCastingBridge.
FOUNDATION_EXPORT const unsigned char MatterTvCastingBridgeVersionString[];

#import "CastingServerBridge.h"

// Add simplified casting API headers here
#import "MCAttribute.h"
#import "MCCastingApp.h"
#import "MCCastingPlayer.h"
#import "MCCastingPlayerDiscovery.h"
#import "MCCluster.h"
#import "MCCommand.h"
#import "MCCommissionableData.h"
#import "MCCryptoUtils.h"
#import "MCDataSource.h"
#import "MCDeviceAttestationCredentials.h"
#import "MCEndpointClusterType.h"
#import "MCEndpointFilter.h"
#import "MCObserver.h"
#import "zap-generated/MCAttributeObjects.h"
#import "zap-generated/MCClusterObjects.h"
#import "zap-generated/MCCommandObjects.h"
