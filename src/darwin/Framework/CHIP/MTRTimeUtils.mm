/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRTimeUtils.h"
#import "MTRConversion.h"
#import "MTRLogging_Internal.h"

NSArray<MTRTimeSynchronizationClusterDSTOffsetStruct *> * _Nullable MTRComputeDSTOffsets(size_t maxCount)
{
    auto * tz = [NSTimeZone localTimeZone];
    if (!tz) {
        MTR_LOG_ERROR("Could not retrieve local time zone. Unable to determine DST offsets.");
        return nil;
    }

    NSMutableArray<MTRTimeSynchronizationClusterDSTOffsetStruct *> * retval = [NSMutableArray arrayWithCapacity:maxCount];

    auto nextOffset = tz.daylightSavingTimeOffset;
    NSNumber * nextValidStarting = @(0);
    auto * nextTransition = tz.nextDaylightSavingTimeTransition;
    for (size_t offsetsAdded = 0; offsetsAdded < maxCount; ++offsetsAdded) {
        auto offset = [[MTRTimeSynchronizationClusterDSTOffsetStruct alloc] init];
        offset.offset = @(nextOffset);
        offset.validStarting = nextValidStarting;
        if (nextTransition == nil) {
            // This one is valid forever.
            offset.validUntil = nil;
        } else {
            uint64_t nextTransitionEpochUs;
            if (!DateToMatterEpochMicroseconds(nextTransition, nextTransitionEpochUs)) {
                // Transition is somehow before Matter epoch start.  This really
                // should not happen, but if it does just don't pretend like we
                // know what's going on with timezones here.
                MTR_LOG_ERROR("Future daylight savings transition is before Matter epoch start?");
                return nil;
            }

            offset.validUntil = @(nextTransitionEpochUs);
        }

        [retval addObject:offset];

        if (offset.validUntil == nil) {
            // Valid forever, so no need for more offsets.
            break;
        }

        nextOffset = [tz daylightSavingTimeOffsetForDate:nextTransition];
        nextValidStarting = offset.validUntil;
        nextTransition = [tz nextDaylightSavingTimeTransitionAfterDate:nextTransition];
    }

    return [retval copy];
}
