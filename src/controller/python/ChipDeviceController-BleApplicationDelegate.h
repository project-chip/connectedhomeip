/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#ifndef DEVICEMANAGER_BLEAPPLICATIONDELEGATE_H_
#define DEVICEMANAGER_BLEAPPLICATIONDELEGATE_H_

#include <ble/BleApplicationDelegate.h>

class DeviceController_BleApplicationDelegate :
public chip::Ble::BleApplicationDelegate
{
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj);
};

#endif /* DEVICEMANAGER_BLEAPPLICATIONDELEGATE_H_ */
