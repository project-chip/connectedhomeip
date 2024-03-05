/**
 *
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

#import "MCCluster_Internal.h"
#import "zap-generated/MCClusterObjects.h"

#import "MCEndpoint_Internal.h"

#import "core/Endpoint.h"

#import <Foundation/Foundation.h>

@implementation MCCluster

- (instancetype _Nonnull)initWithCppCluster:(matter::casting::memory::Strong<matter::casting::core::BaseCluster>)cppCluster
{
    if (self = [super init]) {
        _cppCluster = cppCluster;
    }
    return self;
}

- (MCEndpoint * _Nonnull)endpoint
{
    return [[MCEndpoint alloc] initWithCppEndpoint:_cppCluster->GetEndpoint().lock()];
}

@end
