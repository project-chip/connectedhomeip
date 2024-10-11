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

#import "MCAttribute_Internal.h"
#import "MCCastingApp.h"

#import <Foundation/Foundation.h>

@implementation MCAttribute

- (instancetype _Nonnull)initWithCppAttribute:(void *)cppAttribute
{
    if (self = [super init]) {
        _cppAttribute = cppAttribute;
    }
    return self;
}

- (void)read:(void * _Nullable)context completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong, id _Nullable __strong, NSError * _Nullable __strong))completion
{
}

- (void)subscribe:(void * _Nullable)context completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong, id _Nullable __strong, NSError * _Nullable __strong))completion minInterval:(NSNumber *)minInterval maxInterval:(NSNumber *)maxInterval
{
}

- (void)write:(id _Nonnull)value withCompletionBlock:(void (^_Nonnull __strong)(NSError * _Nullable __strong))completionBlock
{
}

- (id _Nullable)getObjTypeFromCpp:(std::any)cppValue
{
    return nil;
}

@end
