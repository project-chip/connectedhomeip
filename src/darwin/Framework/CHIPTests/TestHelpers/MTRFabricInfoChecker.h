/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

void CheckFabricInfo(NSArray<MTRFabricInfo *> * fabricInfoList, NSMutableSet<NSDictionary *> * expectedSet);

NS_ASSUME_NONNULL_END
