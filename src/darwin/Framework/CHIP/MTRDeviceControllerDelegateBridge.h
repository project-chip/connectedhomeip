/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceControllerDelegate.h"

#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <platform/CHIPDeviceBuildConfig.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

class MTRDeviceControllerDelegateBridge : public chip::Controller::DevicePairingDelegate {
public:
    MTRDeviceControllerDelegateBridge();
    ~MTRDeviceControllerDelegateBridge();

    void setDelegate(MTRDeviceController * controller, id<MTRDeviceControllerDelegate> delegate, dispatch_queue_t queue);

    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;

    void OnPairingComplete(CHIP_ERROR error) override;

    void OnPairingDeleted(CHIP_ERROR error) override;

    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;

    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;

private:
    MTRDeviceController * __weak mController;
    _Nullable id<MTRDeviceControllerDelegate> mDelegate;
    _Nullable dispatch_queue_t mQueue;

    MTRCommissioningStatus MapStatus(chip::Controller::DevicePairingDelegate::Status status);
};

NS_ASSUME_NONNULL_END
