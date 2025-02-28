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

#import "MTRDeviceDataValidation.h"

#import "MTRBaseDevice.h"

#import "MTRLogging_Internal.h"

// MTRDataValueDictionaryIsWellFormed lives in MTRBaseDevice.mm, because it uses
// static functions defined in that file.

#pragma mark - Helpers used by multiple validators

#define MTR_CHECK_CLASS(className) \
    ^(className * arg) { return MTR_SAFE_CAST(arg, className) != nil; }

// input is not known to be an NSDictionary yet on entry.
//
// expectedShape maps keys to value validator blocks.
static BOOL MTRDictionaryHasExpectedShape(NSDictionary * input, NSDictionary * expectedShape)
{
    if (!MTR_SAFE_CAST(input, NSDictionary)) {
        return NO;
    }

    for (id key in expectedShape) {
        id value = input[key];
        if (!value) {
            return NO;
        }
        auto validator = static_cast<BOOL (^)(id)>(expectedShape[key]);
        if (!validator(value)) {
            return NO;
        }
    }

    return YES;
}

#pragma mark - Attribute report validation

static const auto sAttributeDataShape = @{
    MTRAttributePathKey : MTR_CHECK_CLASS(MTRAttributePath),
    MTRDataKey : (^(MTRDeviceDataValueDictionary arg) {
        return MTRDataValueDictionaryIsWellFormed(arg);
    }),
};

static const auto sAttributeErrorShape = @{
    MTRAttributePathKey : MTR_CHECK_CLASS(MTRAttributePath),
    MTRErrorKey : MTR_CHECK_CLASS(NSError),
};

BOOL MTRAttributeReportIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * report)
{
    if (!MTR_SAFE_CAST(report, NSArray)) {
        MTR_LOG_ERROR("Attribute report is not an array: %@", report);
        return NO;
    }

    for (MTRDeviceResponseValueDictionary item in report) {
        // item can be a value report or an error report.
        if (!MTRDictionaryHasExpectedShape(item, sAttributeDataShape) && !MTRDictionaryHasExpectedShape(item, sAttributeErrorShape)) {
            MTR_LOG_ERROR("Attribute report contains a weird entry: %@", item);
            return NO;
        }

        // Now we know item is in fact a dictionary, and it has at least one of MTRDataKey and MTRErrorKey.  Make sure it's
        // not claiming both, which could confuse code that examines it.
        if (item[MTRDataKey] != nil && item[MTRErrorKey] != nil) {
            MTR_LOG_ERROR("Attribute report contains an entry that claims to be both data and error: %@", item);
            return NO;
        }
    }

    return YES;
}

#pragma mark - Event report validation

// MTREventIsHistoricalKey is claimed to be present no matter what, as
// long as MTREventPathKey is present.
static const auto sEventDataShape = @{
    MTREventPathKey : MTR_CHECK_CLASS(MTREventPath),
    MTRDataKey : (^(MTRDeviceDataValueDictionary arg) {
        return MTRDataValueDictionaryIsWellFormed(arg);
    }),
    MTREventIsHistoricalKey : MTR_CHECK_CLASS(NSNumber),
    MTREventNumberKey : MTR_CHECK_CLASS(NSNumber),
    MTREventPriorityKey : MTR_CHECK_CLASS(NSNumber),
    MTREventTimeTypeKey : MTR_CHECK_CLASS(NSNumber),
};

static const auto sEventErrorShape = @{
    MTREventPathKey : MTR_CHECK_CLASS(MTREventPath),
    MTRErrorKey : MTR_CHECK_CLASS(NSError),
    MTREventIsHistoricalKey : MTR_CHECK_CLASS(NSNumber),
};

BOOL MTREventReportIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * report)
{
    if (!MTR_SAFE_CAST(report, NSArray)) {
        MTR_LOG_ERROR("Event report is not an array: %@", report);
        return NO;
    }

    for (MTRDeviceResponseValueDictionary item in report) {
        // item can be a value report or an error report.
        if (!MTRDictionaryHasExpectedShape(item, sEventDataShape) && !MTRDictionaryHasExpectedShape(item, sEventErrorShape)) {
            MTR_LOG_ERROR("Event report contains a weird entry: %@", item);
            return NO;
        }

        // Now we know item is in fact a dictionary, and it has at least one of MTRDataKey and MTRErrorKey.  Make sure it's
        // not claiming both, which could confuse code that examines it.
        if (item[MTRDataKey] != nil && item[MTRErrorKey] != nil) {
            MTR_LOG_ERROR("Event report contains an entry that claims to be both data and error: %@", item);
            return NO;
        }

        if (item[MTRDataKey]) {
            // Check well-formedness of our timestamps.  Note that we have
            // already validated the type of item[MTREventTimeTypeKey].
            uint64_t eventTimeType = [item[MTREventTimeTypeKey] unsignedLongLongValue];
            switch (eventTimeType) {
            case MTREventTimeTypeSystemUpTime: {
                if (!MTR_SAFE_CAST(item[MTREventSystemUpTimeKey], NSNumber)) {
                    MTR_LOG_ERROR("Event report claims system uptime timing but does not have the time: %@", item);
                    return NO;
                }
                break;
            }
            case MTREventTimeTypeTimestampDate: {
                if (!MTR_SAFE_CAST(item[MTREventTimestampDateKey], NSDate)) {
                    MTR_LOG_ERROR("Event report claims epoch timing but does not have the time: %@", item);
                    return NO;
                }
                break;
            }
            default:
                MTR_LOG_ERROR("Unknown time type for event report: %@", item);
                return NO;
            }
        }
    }

    return YES;
}

#pragma mark - Invoke response validation

BOOL MTRInvokeResponseIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * response)
{
    if (!MTRInvokeResponsesAreWellFormed(response)) {
        return NO;
    }

    // Input is an array with a single value.
    if (response.count != 1) {
        MTR_LOG_ERROR("Invoke response is not an array with exactly one entry: %@", response);
        return NO;
    }

    return YES;
}

BOOL MTRInvokeResponsesAreWellFormed(NSArray<MTRDeviceResponseValueDictionary> * responses)
{
    if (!MTR_SAFE_CAST(responses, NSArray)) {
        MTR_LOG_ERROR("Invoke responses are not an array: %@", responses);
        return NO;
    }

    for (MTRDeviceResponseValueDictionary responseValue in responses) {
        // Each entry must be a dictionary that has MTRCommandPathKey.

        if (!MTR_SAFE_CAST(responseValue, NSDictionary) || !MTR_SAFE_CAST(responseValue[MTRCommandPathKey], MTRCommandPath)) {
            MTR_LOG_ERROR("Invoke response has an invalid array entry: %@", responseValue);
            return NO;
        }

        MTRDeviceDataValueDictionary _Nullable data = responseValue[MTRDataKey];
        NSError * _Nullable error = responseValue[MTRErrorKey];

        if (data != nil && error != nil) {
            MTR_LOG_ERROR("Invoke response claims to have both data and error: %@", responseValue);
            return NO;
        }

        if (error != nil) {
            if (!MTR_SAFE_CAST(error, NSError)) {
                MTR_LOG_ERROR("Invoke response %@ has %@ instead of an NSError", responseValue, error);
                return NO;
            }

            // Valid error response.
            continue;
        }

        if (data == nil) {
            // This is valid: indicates a success status response.
            continue;
        }

        if (!MTRDataValueDictionaryIsWellFormed(data)) {
            MTR_LOG_ERROR("Invoke response claims to have data that is not a data-value: %@", data);
            return NO;
        }

        // Now we know data is a dictionary (in fact a data-value).  The only thing
        // we promise about it is that it has type MTRStructureValueType.
        if (![MTRStructureValueType isEqual:data[MTRTypeKey]]) {
            MTR_LOG_ERROR("Invoke response data is not of structure type: %@", data);
            return NO;
        }
    }

    return YES;
}
