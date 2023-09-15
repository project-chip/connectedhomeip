/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#ifndef MatterError_h
#define MatterError_h

@interface MatterError : NSObject

@property uint32_t code;

@property NSString * _Nullable message;

extern MatterError * _Nonnull MATTER_NO_ERROR;
extern MatterError * _Nonnull MATTER_ERROR_INCORRECT_STATE;
extern MatterError * _Nonnull MATTER_ERROR_INVALID_ARGUMENT;

- (MatterError * _Nonnull)initWithCode:(uint32_t)code message:(NSString * _Nullable)message;

@end

#endif /* MatterError_h */
