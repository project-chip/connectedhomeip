/**
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

#import "MTRDevice_XPC.h"
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRDevice_XPC ()
@property (nullable, atomic, readwrite, retain, getter=_internalState, setter=_setInternalState:) NSDictionary * _internalState;
@end

NS_ASSUME_NONNULL_END
