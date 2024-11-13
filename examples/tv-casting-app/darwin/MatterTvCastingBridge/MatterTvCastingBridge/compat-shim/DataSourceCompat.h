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

#import "MCDataSource.h"

#import "AppParameters.h"

#import <Foundation/Foundation.h>

#ifndef DataSourceCompat_h
#define DataSourceCompat_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface DataSourceCompat : NSObject<MCDataSource>

- (instancetype)initWithClientQueue:(dispatch_queue_t)clientQueue;
- (void)setAppParameters:(AppParameters *)appParameters;
- (void)setDacHolder:(DeviceAttestationCredentialsHolder *)dacHolder;

@end

#endif /* DataSourceCompat_h */
