/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
