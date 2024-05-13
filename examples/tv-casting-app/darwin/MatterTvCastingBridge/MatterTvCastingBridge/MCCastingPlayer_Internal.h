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

#import "MCCastingPlayer.h"

#import "core/CastingPlayer.h"

#import <Foundation/Foundation.h>

#ifndef MCCastingPlayer_Internal_h
#define MCCastingPlayer_Internal_h

@interface MCCastingPlayer ()

+ (MCCastingPlayer * _Nullable)getTargetCastingPlayer;

- (instancetype _Nonnull)initWithCppCastingPlayer:(matter::casting::memory::Strong<matter::casting::core::CastingPlayer>)cppCastingPlayer;

@end

#endif /* MCCastingPlayer_Internal_h */
