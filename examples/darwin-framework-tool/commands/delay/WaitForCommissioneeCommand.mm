/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WaitForCommissioneeCommand.h"

#import "MTRDevice_Externs.h"

@implementation MTRDeviceTestDelegate
- (instancetype)initWithCommandBridge:(CHIPCommandBridge *)commandBridge
{
    if (!(self = [super init])) {
        return nil;
    }

    _commandBridge = commandBridge;
    return self;
}

- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
    if (state == MTRDeviceStateReachable) {
        _commandBridge->SetCommandExitStatus(CHIP_NO_ERROR);
    } else if (state == MTRDeviceStateUnreachable) {
        _commandBridge->SetCommandExitStatus(CHIP_ERROR_NOT_FOUND);
    } else if (state == MTRDeviceStateUnknown) {
        _commandBridge->SetCommandExitStatus(CHIP_ERROR_NOT_FOUND);
    } else {
        // This should not happens.
        chipDie();
    }
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
}
@end

CHIP_ERROR WaitForCommissioneeCommand::RunCommand()
{
    MTRDeviceController * commissioner = CurrentCommissioner();

    auto * base_device = [MTRBaseDevice deviceWithNodeID:@(mNodeId) controller:commissioner];
    VerifyOrReturnError(base_device != nil, CHIP_ERROR_INCORRECT_STATE);

    if (mExpireExistingSession.ValueOr(true)) {
        [base_device invalidateCASESession];
    }
    base_device = nil;

    auto * device = [MTRDevice deviceWithNodeID:@(mNodeId) controller:commissioner];
    VerifyOrReturnError(device != nil, CHIP_ERROR_INCORRECT_STATE);

    auto queue = dispatch_queue_create("com.chip.wait_for_commissionee", DISPATCH_QUEUE_SERIAL);
    [device setDelegate:mDeviceDelegate queue:queue];

    return CHIP_NO_ERROR;
}
