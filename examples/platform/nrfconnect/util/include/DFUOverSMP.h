/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Provides an implementation of Device Firmware Upgrade using SMP protocol
 *      over Bluetooth LE functionality for nRF Connect SDK platform.
 */

#pragma once

#include <platform/CHIPDeviceLayer.h>

#include <zephyr/zephyr.h>

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

    static int UploadConfirmHandler(const struct img_mgmt_upload_req req, const struct img_mgmt_upload_action action);
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
