/**
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

#import <Foundation/Foundation.h>

#import "CHIPErrorTestUtils.h"

#import <CHIP/CHIP.h>

#import <app/util/af-enums.h>

NS_ASSUME_NONNULL_BEGIN

@implementation CHIPErrorTestUtils
+ (uint8_t)errorToZCLErrorCode:(NSError * _Nullable)error
{
    // This duplicates CHIPError's errorToZCLErrorCode, but I can't
    // figure out a way to include/use that here.
    if (error == nil) {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    if (error.domain != CHIPErrorDomain) {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    switch (error.code) {
    case CHIPErrorCodeDuplicateExists:
        return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
    case CHIPErrorCodeUnsupportedEndpoint:
        return EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT;
    case CHIPErrorCodeUnsupportedCommand:
        return EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND;
    case CHIPErrorCodeInvalidCommand:
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    case CHIPErrorCodeUnsupportedAttribute:
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    case CHIPErrorCodeConstraintError:
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    case CHIPErrorCodeUnsupportedWrite:
        return EMBER_ZCL_STATUS_UNSUPPORTED_WRITE;
    case CHIPErrorCodeUnsupportedCluster:
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    case CHIPSuccess:
        return EMBER_ZCL_STATUS_SUCCESS;
    default:
        return EMBER_ZCL_STATUS_FAILURE;
    }
}
@end

NS_ASSUME_NONNULL_END
