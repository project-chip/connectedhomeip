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

#import "CHIPDeviceStatusDelegate.h"
#import <Foundation/Foundation.h>

#include <controller/CHIPDevice.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPDeviceStatusDelegateBridge : public chip::Controller::DeviceStatusDelegate
{
public:
    CHIPDeviceStatusDelegateBridge();
    ~CHIPDeviceStatusDelegateBridge();

    void setDelegate(id<CHIPDeviceStatusDelegate> delegate, dispatch_queue_t queue);

    void OnMessage(chip::System::PacketBufferHandle message) override;

    void OnStatusChange() override;

private:
    id<CHIPDeviceStatusDelegate> mDelegate;
    dispatch_queue_t mQueue;
};

NS_ASSUME_NONNULL_END
