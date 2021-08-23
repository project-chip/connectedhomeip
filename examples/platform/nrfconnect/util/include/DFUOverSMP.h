/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      Provides an implementation of Device Firmware Upgrade using SMP protocol
 *      over Bluetooth LE functionality for nRF Connect SDK platform.
 */

#pragma once

#include <platform/CHIPDeviceLayer.h>

#include <zephyr.h>

typedef void (*DFUOverSMPRestartAdvertisingHandler)(void);

class DFUOverSMP
{
public:
    void Init(DFUOverSMPRestartAdvertisingHandler startAdvertisingCb);
    void ConfirmNewImage();
    void StartServer();
    void StartBLEAdvertising();
    bool IsEnabled() { return mIsEnabled; }

private:
    friend DFUOverSMP & GetDFUOverSMP(void);

    static int UploadConfirmHandler(uint32_t offset, uint32_t size, void * arg);
    static void OnBleDisconnect(bt_conn * conn, uint8_t reason);
    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    bool mIsEnabled;
    bool mIsAdvertisingEnabled;
    bt_conn_cb mBleConnCallbacks;
    DFUOverSMPRestartAdvertisingHandler restartAdvertisingCallback;

    static DFUOverSMP sDFUOverSMP;
};

inline DFUOverSMP & GetDFUOverSMP(void)
{
    return DFUOverSMP::sDFUOverSMP;
}
