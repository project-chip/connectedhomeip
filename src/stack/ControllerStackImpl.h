/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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

#pragma once

#include <ble/BleLayer.h>
#include <stack/ControllerStack.h>
#include <stack/StackImpl.h>

namespace chip {

/* ========================== ControllerStackImpl ========================== */
template<typename... Configurations>
class ControllerStackImpl : public ControllerStack, public StackImpl<Configurations...> {
public:
    ControllerStackImpl(NodeId localDeviceId) : StackImpl<Configurations...>(localDeviceId) {}
    ~ControllerStackImpl() override {}

    CHIP_ERROR InitController(PersistentStorageDelegate * storageDelegate, Controller::DevicePairingDelegate * pairingDelegate) override
    {
        ReturnErrorOnFailure(StackImpl<Configurations...>::Init());
        ReturnErrorOnFailure(mDeviceCommissioner.Init(this, storageDelegate, pairingDelegate));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Shutdown() override {
        ReturnErrorOnFailure(mDeviceCommissioner.Shutdown());
        ReturnErrorOnFailure(StackImpl<Configurations...>::Shutdown());
        return CHIP_NO_ERROR;
    }

    Controller::DeviceCommissioner & GetDeviceCommissioner() override { return mDeviceCommissioner; }

private:
    Controller::DeviceCommissioner mDeviceCommissioner;
};

} // namespace chip
