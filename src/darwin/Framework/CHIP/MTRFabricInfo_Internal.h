/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRFabricInfo.h>

#include <credentials/FabricTable.h>

@interface MTRFabricInfo ()
- (instancetype)initWithFabricTable:(const chip::FabricTable &)fabricTable fabricInfo:(const chip::FabricInfo &)fabricInfo;
@end
