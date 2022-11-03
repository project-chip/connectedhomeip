/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDevicePairingDelegate.h"

#include <controller/CHIPDeviceController.h>
#include <platform/CHIPDeviceBuildConfig.h>

NS_ASSUME_NONNULL_BEGIN

class MTRDevicePairingDelegateBridge : public chip::Controller::DevicePairingDelegate
{
public:
    MTRDevicePairingDelegateBridge();
    ~MTRDevicePairingDelegateBridge();

    void setDelegate(id<MTRDevicePairingDelegate> delegate, dispatch_queue_t queue);

    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;

    void OnPairingComplete(CHIP_ERROR error) override;

    void OnPairingDeleted(CHIP_ERROR error) override;

    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;

private:
    _Nullable id<MTRDevicePairingDelegate> mDelegate;
    _Nullable dispatch_queue_t mQueue;

    MTRPairingStatus MapStatus(chip::Controller::DevicePairingDelegate::Status status);
};

NS_ASSUME_NONNULL_END
