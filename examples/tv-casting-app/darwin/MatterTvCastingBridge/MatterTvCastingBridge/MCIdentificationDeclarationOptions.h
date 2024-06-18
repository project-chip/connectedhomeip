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
#import "MCTargetAppInfo.h"
#import <Foundation/Foundation.h>

#ifndef MCIdentificationDeclarationOptions_h
#define MCIdentificationDeclarationOptions_h

/**
 * This class contains the optional parameters used in the IdentificationDeclaration Message, sent
 * by the Commissionee to the Commissioner. The options specify information relating to the
 * requested UDC commissioning session.
 */
@interface MCIdentificationDeclarationOptions : NSObject

- (instancetype)init;

- (instancetype)initWithCommissionerPasscodeOnly:(BOOL)commissionerPasscode;

// Getter methods
- (BOOL)getNoPasscode;
- (BOOL)getCdUponPasscodeDialog;
- (BOOL)getCommissionerPasscode;
- (BOOL)getCommissionerPasscodeReady;
- (BOOL)getCancelPasscode;

/**
 * @brief Adds a TargetAppInfo to the IdentificationDeclarationOptions.java TargetAppInfos list,
 *     up to a maximum of CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS.
 */
- (BOOL)addTargetAppInfo:(MCTargetAppInfo *)targetAppInfo NS_SWIFT_NAME(addTargetAppInfo(_:));
- (NSArray<MCTargetAppInfo *> *)getTargetAppInfoList;

- (NSString *)description;
- (void)logDetail;

@end

#endif /* MCIdentificationDeclarationOptions_h */
