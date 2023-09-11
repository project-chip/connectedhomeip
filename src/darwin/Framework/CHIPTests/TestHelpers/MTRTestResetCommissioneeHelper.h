/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

#pragma once

void ResetCommissionee(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * testcase, uint16_t commandTimeout);
