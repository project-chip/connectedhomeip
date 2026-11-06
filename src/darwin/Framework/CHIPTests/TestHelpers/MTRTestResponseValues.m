/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import "MTRTestResponseValues.h"

#import <Matter/Matter.h>

NSDictionary<NSString *, id> * TestAttributeResponseValue(NSNumber * endpointID, NSNumber * clusterID, NSNumber * attributeID, unsigned int testValue)
{
    return @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID],
        MTRDataKey : @ {
            MTRDataVersionKey : @(testValue),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(testValue),
        }
    };
}

NSDictionary<NSString *, id> * TestEventResponseValue(NSNumber * endpointID, NSNumber * clusterID, NSNumber * eventID)
{
    // Pick a large event number, so that in practice these event reports will
    // all have larger event numbers than anything that might have been observed
    // before.
    static uint64_t eventNumber = 0x100000000llu;
    return @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:endpointID clusterID:clusterID eventID:eventID],
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTREventIsHistoricalKey : @(NO),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventNumberKey : @(eventNumber++),
        // Empty payload.
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };
}

@implementation MTRInterestedPathsTestClusterPath
@end

@implementation MTRInterestedPathsTestAttributePath
@end

@implementation MTRInterestedPathsTestEventPath
@end

id MTRInterestedPathsTestPathAsSubclass(MTRClusterPath * path, Class subclass)
{
    NSKeyedArchiver * archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [archiver setClassName:NSStringFromClass(subclass) forClass:[path class]];
    [archiver encodeObject:path forKey:NSKeyedArchiveRootObjectKey];
    [archiver finishEncoding];
    return [NSKeyedUnarchiver unarchivedObjectOfClass:subclass fromData:archiver.encodedData error:nil];
}
