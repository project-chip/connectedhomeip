/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRClusterConstants_Private.h"
#import <Matter/MTRClusterNames.h>

// @implementation MTRClusterAttributeIDType

#pragma mark - Clusters IDs

NSString * MTRPrivateClusterNameForID(MTRClusterIDType clusterID)
{
    NSString * result = nil;

    switch ((MTRPrivateClusterIDType) clusterID) {

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Attributes IDs

NSString * MTRPrivateAttributeNameForID(MTRClusterIDType clusterID, MTRAttributeIDType attributeID)
{
    NSString * result = nil;

    switch ((MTRPrivateClusterIDType) clusterID) {

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Command IDs

NSString * MTRPrivateRequestCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID)
{
    NSString * result = nil;

    switch ((MTRPrivateClusterIDType) clusterID) {

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

NSString * MTRPrivateResponseCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID)
{
    NSString * result = nil;

    switch ((MTRPrivateClusterIDType) clusterID) {

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Event IDs

NSString * MTRPrivateEventNameForID(MTRClusterIDType clusterID, MTREventIDType eventID)
{
    NSString * result = nil;

    switch ((MTRPrivateClusterIDType) clusterID) {

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

// @end
