/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#import <Matter/MTRDefines.h>

#import "MTRCommissionableBrowserResult.h"

#include <controller/SetUpCodePairer.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRCommissionableBrowserResultInterfaces : NSObject

@property (nonatomic, readwrite) chip::Optional<chip::Dnssd::CommonResolutionData> resolutionData;

@end

@interface MTRCommissionableBrowserResult ()

@property (nonatomic, readwrite) NSMutableDictionary<NSNumber *, MTRCommissionableBrowserResultInterfaces *> * interfaces;
@property (nonatomic, readwrite) chip::Optional<chip::Controller::SetUpCodePairerParameters> params;

@end

NS_ASSUME_NONNULL_END
