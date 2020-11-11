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

#import "CHIPDevicePairingDelegate.h"
#import <Foundation/Foundation.h>

#include <controller/CHIPDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPDevicePairingDelegateBridge : public chip::Controller::DevicePairingDelegate
{
public:
    CHIPDevicePairingDelegateBridge();
    ~CHIPDevicePairingDelegateBridge();

    void setDelegate(id<CHIPDevicePairingDelegate> delegate, dispatch_queue_t queue);

    void OnStatusUpdate(chip::RendezvousSessionDelegate::Status status) override;

    void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback) override;

    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           chip::RendezvousDeviceCredentialsDelegate * callback) override;

    void OnPairingComplete(CHIP_ERROR error) override;

    void OnPairingDeleted(CHIP_ERROR error) override;

private:
    id<CHIPDevicePairingDelegate> mDelegate;
    dispatch_queue_t mQueue;

    SendNetworkCredentials mHandler;
    chip::RendezvousDeviceCredentialsDelegate * mCallback;

    PairingStatus MapStatus(chip::RendezvousSessionDelegate::Status status);
};

NS_ASSUME_NONNULL_END
