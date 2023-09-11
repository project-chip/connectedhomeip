/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPError.h>

#include <Matter/Matter.h>

CHIP_ERROR MTRErrorToCHIPErrorCode(NSError * error);
id NSObjectFromCHIPTLV(chip::TLV::TLVReader * data);
