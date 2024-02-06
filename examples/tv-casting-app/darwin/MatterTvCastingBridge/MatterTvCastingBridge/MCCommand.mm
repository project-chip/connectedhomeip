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

#import "MCCastingApp.h"
#import "MCCommand_Internal.h"

#import <Foundation/Foundation.h>

@implementation MCCommand

- (instancetype _Nonnull)initWithCppCommand:(void *)cppCommand
{
    if (self = [super init]) {
        _cppCommand = cppCommand;
    }
    return self;
}

- (void)invoke:(id _Nonnull)request
                 context:(void * _Nullable)context
              completion:(void (^_Nonnull __strong)(void * _Nullable, NSError * _Nullable __strong, id _Nullable __strong))completion
    timedInvokeTimeoutMs:(NSNumber * _Nullable)timedInvokeTimeoutMs
{
}

- (std::any)getCppRequestFromObjC:(id _Nonnull)objcRequest
{
    return nil;
}

- (id _Nullable)getObjCResponseFromCpp:(std::any)cppResponse
{
    return nil;
}

@end
