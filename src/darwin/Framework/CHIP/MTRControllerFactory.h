/**
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

#import <Foundation/Foundation.h>

#import <Matter/MTRDeviceControllerFactory.h>

NS_ASSUME_NONNULL_BEGIN

API_DEPRECATED("Please use MTRDeviceControllerFactory", ios(16.1, 16.2), macos(13.0, 13.1), watchos(9.1, 9.2), tvos(16.1, 16.2))
@interface MTRControllerFactory : MTRDeviceControllerFactory
+ (instancetype)sharedInstance;
- (BOOL)startup:(MTRControllerFactoryParams *)startupParams;
- (void)shutdown;
- (MTRDeviceController * _Nullable)startControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams;
- (MTRDeviceController * _Nullable)startControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams;
@end

NS_ASSUME_NONNULL_END
