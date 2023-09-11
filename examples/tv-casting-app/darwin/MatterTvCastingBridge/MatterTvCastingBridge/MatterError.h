/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
