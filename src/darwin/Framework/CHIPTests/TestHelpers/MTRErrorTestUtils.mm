/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRErrorTestUtils.h"

#import <Matter/Matter.h>

#import <app/util/af-enums.h>

NS_ASSUME_NONNULL_BEGIN

@implementation MTRErrorTestUtils
+ (uint8_t)errorToZCLErrorCode:(NSError * _Nullable)error
{
    // This duplicates MTRError's errorToZCLErrorCode, but I can't
    // figure out a way to include/use that here.
    if (error == nil) {
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    if (![error.domain isEqualToString:MTRInteractionErrorDomain]) {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return static_cast<uint8_t>(error.code);
}
@end

NS_ASSUME_NONNULL_END
