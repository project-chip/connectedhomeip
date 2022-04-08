/**
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

#import "CHIPDeviceControllerOverXPC.h"
#import "CHIPDeviceControllerXPCConnection.h"

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceControllerOverXPC ()

@property (nonatomic, readwrite, strong) id<NSCopying> _Nullable controllerId;
@property (nonatomic, readonly, strong) dispatch_queue_t workQueue;
@property (nonatomic, readonly, strong) CHIPDeviceControllerXPCConnection * xpcConnection;

@end

NS_ASSUME_NONNULL_END
