/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

#import "MTRCommissionableBrowserResult.h"

#include <controller/SetUpCodePairer.h>

NS_ASSUME_NONNULL_BEGIN

MTR_HIDDEN
@interface MTRCommissionableBrowserResultInterfaces : NSObject

@property (nonatomic, readwrite) chip::Optional<chip::Dnssd::CommonResolutionData> resolutionData;

@end

@interface MTRCommissionableBrowserResult ()

@property (nonatomic, readwrite) NSMutableDictionary<NSNumber *, MTRCommissionableBrowserResultInterfaces *> * interfaces;
@property (nonatomic, readwrite) chip::Optional<chip::Controller::SetUpCodePairerParameters> params;

@end

NS_ASSUME_NONNULL_END
