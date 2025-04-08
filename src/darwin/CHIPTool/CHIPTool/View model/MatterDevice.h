/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

NS_ASSUME_NONNULL_BEGIN

@interface MatterDevice : NSObject

@property (assign, nonatomic) uint64_t nodeId;

@property (strong, nonatomic) NSNumber * recoveryId;

@property (strong, nonatomic) NSString * produceName;

@property (strong, nonatomic) NSNumber * deviceType;

@property (assign, nonatomic) bool onOff;

@property (assign, nonatomic) bool isNetworkRecoverable;

-(instancetype)initWithNodeId:(uint64_t)nodeId;

@end

NS_ASSUME_NONNULL_END
