//
/**
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
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

MTR_EXTERN MTR_EXPORT @interface MTRDeviceControllerLocalTestStorage : NSObject<MTRDeviceControllerStorageDelegate>

// Setting this variable only affects subsequent MTRDeviceController initializations
@property (class, nonatomic, assign) BOOL localTestStorageEnabled;

// This storage persists items to NSUserDefaults for MTRStorageSharingTypeNotShared data. Items with other sharing types will be droppped, or stored/fetched with the "passthrough storage" if one is specified.
- (instancetype)initWithPassThroughStorage:(id<MTRDeviceControllerStorageDelegate> _Nullable)passThroughStorage;

@end

NS_ASSUME_NONNULL_END
