/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ChipDeviceScanner.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <errno.h>
#include <lib/support/logging/CHIPLogging.h>
#include <pthread.h>

#include "MainLoop.h"

#include <pbnjson.h>

#include <string>

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace {

static unsigned int kScanTimeout = 10000;

// Default CHIP Scan Timeout in Millisecond
// static unsigned int kScanTimeout = 10000;

struct GObjectUnref
{
    template <typename T>
    void operator()(T * value)
    {
        g_object_unref(value);
    }
};

using GCancellableUniquePtr       = std::unique_ptr<GCancellable, GObjectUnref>;
using GDBusObjectManagerUniquePtr = std::unique_ptr<GDBusObjectManager, GObjectUnref>;

} // namespace

ChipDeviceScanner::ChipDeviceScanner(LSHandle * handle, ChipDeviceScannerDelegate * delegate) :
    mLSHandle(handle), mDelegate(delegate)
{}

ChipDeviceScanner::ChipDeviceScanner(ChipDeviceScannerDelegate * delegate) : mDelegate(delegate) {}

ChipDeviceScanner::~ChipDeviceScanner()
{
    StopScan();

    // In case the timeout timer is still active
    chip::DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, this);
    mDelegate = nullptr;
}

std::unique_ptr<ChipDeviceScanner> ChipDeviceScanner::Create(LSHandle * handle, ChipDeviceScannerDelegate * delegate)
{
    if (!LSCall(handle, "palm://com.webos.service.bluetooth2/adapter/getStatus", "{}", NULL, NULL, NULL, NULL))
    {
        g_print("Failed to call getStatus LSCall\n");
    }

    return std::make_unique<ChipDeviceScanner>(handle, delegate);
}

std::unique_ptr<ChipDeviceScanner> ChipDeviceScanner::Create(ChipDeviceScannerDelegate * delegate)
{
    return std::make_unique<ChipDeviceScanner>(delegate);
}

gboolean ChipDeviceScanner::TimerExpiredCb(gpointer userData)
{
    ChipDeviceScanner * self = (ChipDeviceScanner *) userData;
    ChipLogProgress(DeviceLayer, "Scan Timer expired!!");
    self->StopChipScan();
    return G_SOURCE_REMOVE;
}

void ChipDeviceScanner::printFoundChipDevice(const jvalue_ref & scanRecord, const std::string & address)
{

    int j = 0;

    int scanRecordLength = jarray_size(scanRecord);

    printf("printFoundChipDevice start : scanRecoredLength : %d \n", scanRecordLength);
    while (j < scanRecordLength)
    {
        int32_t l  = -1;
        int32_t t  = -1;
        int32_t v0 = -1;
        int32_t v1 = -1;

        jvalue_ref lObj  = jarray_get(scanRecord, j);
        jvalue_ref tObj  = jarray_get(scanRecord, j + 1);
        jvalue_ref v0Obj = jarray_get(scanRecord, j + 2);
        jvalue_ref v1Obj = jarray_get(scanRecord, j + 3);

        jnumber_get_i32(lObj, &l);
        jnumber_get_i32(tObj, &t);
        jnumber_get_i32(v0Obj, &v0);
        jnumber_get_i32(v1Obj, &v1);

        if (t == 22 && v0 == 175 && v1 == 254)
        { // 22 = 0x16  175 = 0xAF,  254 = 0xFE

            int32_t disc1 = -1;
            int32_t disc2 = -1;

            jvalue_ref disc1Obj = jarray_get(scanRecord, j + 5);
            jvalue_ref disc2Obj = jarray_get(scanRecord, j + 6);

            jnumber_get_i32(disc1Obj, &disc1);
            jnumber_get_i32(disc2Obj, &disc2);

            // uint16_t discriminator = (disc2 << 8) | disc1;

            int32_t vid1 = -1;
            int32_t vid2 = -1;
            int32_t pid1 = -1;
            int32_t pid2 = -1;

            jvalue_ref vid1Obj = jarray_get(scanRecord, j + 7);
            jvalue_ref vid2Obj = jarray_get(scanRecord, j + 8);
            jvalue_ref pid1Obj = jarray_get(scanRecord, j + 9);
            jvalue_ref pid2Obj = jarray_get(scanRecord, j + 10);

            jnumber_get_i32(vid1Obj, &vid1);
            jnumber_get_i32(vid2Obj, &vid2);
            jnumber_get_i32(pid1Obj, &pid1);
            jnumber_get_i32(pid2Obj, &pid2);

            // int32_t vid = (vid2 << 8) | vid1;
            // int32_t pid = (pid2 << 8) | pid1;

            return;
        }
        j += l + 1;
    }
}

bool ChipDeviceScanner::deviceGetstatusCb(LSHandle * sh, LSMessage * message, void * userData)
{
    ChipDeviceScanner * self = (ChipDeviceScanner *) userData;

    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return true;

    jvalue_ref devicesObj = { 0 };

    if (jobject_get_exists(parsedObj, J_CSTR_TO_BUF("devices"), &devicesObj))
    {
        int devicesLength = jarray_size(devicesObj);
        int i             = 0;
        while (i < devicesLength)
        {
            jvalue_ref devicesElementObj = jarray_get(devicesObj, i);

            jvalue_ref manufacturerDataObj = { 0 };

            if (jobject_get_exists(devicesElementObj, J_CSTR_TO_BUF("manufacturerData"), &manufacturerDataObj))
            {
                jvalue_ref scanRecordObj = { 0 };

                if (jobject_get_exists(manufacturerDataObj, J_CSTR_TO_BUF("scanRecord"), &scanRecordObj))
                {
                    int scanRecordLength = jarray_size(scanRecordObj);

                    int j = 0;
                    while (j < scanRecordLength)
                    {
                        jvalue_ref scanRecordElementObj = jarray_get(scanRecordObj, j);

                        int32_t scanRecordElement = -1;

                        jnumber_get_i32(scanRecordElementObj, &scanRecordElement);

                        if (scanRecordElement == 3)
                        {
                            int32_t firstByte  = -1;
                            int32_t secondByte = -1;
                            int32_t thirdByte  = -1;

                            jvalue_ref firstByteObj  = jarray_get(scanRecordObj, j + 1);
                            jvalue_ref secondByteObj = jarray_get(scanRecordObj, j + 2);
                            jvalue_ref thirdByteObj  = jarray_get(scanRecordObj, j + 3);

                            jnumber_get_i32(firstByteObj, &firstByte);
                            jnumber_get_i32(secondByteObj, &secondByte);
                            jnumber_get_i32(thirdByteObj, &thirdByte);

                            if (firstByte == 3 && secondByte == 246 && thirdByte == 255)
                            {
                                jvalue_ref addressObj = { 0 };
                                if (jobject_get_exists(devicesElementObj, J_CSTR_TO_BUF("address"), &addressObj))
                                {
                                    raw_buffer address_buf = jstring_get(addressObj);
                                    char * address         = g_strdup(address_buf.m_str);
                                    jstring_free_buffer(address_buf);

                                    printFoundChipDevice(scanRecordObj, address);

                                    self->mDelegate->OnChipDeviceScanned(address);
                                }

                                jvalue_ref nameObj = { 0 };
                                if (jobject_get_exists(devicesElementObj, J_CSTR_TO_BUF("name"), &nameObj))
                                {
                                    raw_buffer name_buf = jstring_get(nameObj);
                                    char * name         = g_strdup(name_buf.m_str);
                                    jstring_free_buffer(name_buf);

                                    printf("name : %s \n", name);
                                }
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }

                        j = j + scanRecordElement + 1;
                    }
                }
            }
            i = i + 1;
        }
    }

    return true;
}

bool ChipDeviceScanner::startDiscoveryCb(LSHandle * sh, LSMessage * message, void * userData)
{
    return true;
}

gboolean ChipDeviceScanner::TriggerScan(GMainLoop * mainLoop, gpointer userData)
{
    ChipDeviceScanner * self = (ChipDeviceScanner *) userData;
    int ret                  = 0;
    GSource * idleSource;

    self->mAsyncLoop = mainLoop;

    ret = LSCall(self->mLSHandle, "luna://com.webos.service.bluetooth2/adapter/internal/startDiscovery",
                 "{\"typeOfDevice\":\"ble\"}", startDiscoveryCb, userData, NULL, NULL);

    ret = LSCall(self->mLSHandle, "luna://com.webos.service.bluetooth2/device/getStatus", "{\"subscribe\":true}", deviceGetstatusCb,
                 userData, NULL, NULL);

    VerifyOrExit(ret == 1, ChipLogError(DeviceLayer, "bt_adapter_le_start_scan() ret: %d", ret));
    ChipLogProgress(DeviceLayer, "Scan started");

    // Start Timer
    idleSource = g_timeout_source_new(kScanTimeout);
    g_source_set_callback(idleSource, TimerExpiredCb, userData, nullptr);
    g_source_set_priority(idleSource, G_PRIORITY_HIGH_IDLE);
    g_source_attach(idleSource, g_main_loop_get_context(self->mAsyncLoop));
    g_source_unref(idleSource);
    return true;

exit:
    return false;
}

CHIP_ERROR ChipDeviceScanner::StartChipScan(unsigned timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    kScanTimeout = timeoutMs;

    mLSHandle = MainLoop::Instance().mLSHandle;

    // All set to trigger LE Scan
    ChipLogProgress(DeviceLayer, "Start CHIP Scan...");
    if (MainLoop::Instance().AsyncRequest(TriggerScan, this) == false)
    {
        ChipLogError(DeviceLayer, "Failed to trigger Scan...");
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    mIsScanning = true; // optimistic, to allow all callbacks to check this
    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "Start CHIP Scan could not succeed fully! Stop Scan...");
    StopChipScan();
    // UnRegisterScanFilter();
    return err;
}

bool ChipDeviceScanner::cancelDiscoveryCb(LSHandle * sh, LSMessage * message, void * userData)
{
    return true;
}

CHIP_ERROR ChipDeviceScanner::StopChipScan()
{
    int ret = 0;
    ReturnErrorCodeIf(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    ret = LSCall(mLSHandle, "luna://com.webos.service.bluetooth2/dadapter/cancelDiscovery", "{}", cancelDiscoveryCb, this, NULL,
                 NULL);

    ChipLogError(DeviceLayer, "Stop CHIP scan ret: %d", ret);

    g_main_loop_quit(mAsyncLoop);
    ChipLogProgress(DeviceLayer, "CHIP Scanner Async Thread Quit Done..Wait for Thread Windup...!");

    // Report to Impl class
    mDelegate->OnScanComplete();

    mIsScanning = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StartScan(System::Clock::Timeout timeout)
{
    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::TimerExpiredCallback(chip::System::Layer * layer, void * appState)
{
    static_cast<ChipDeviceScanner *>(appState)->StopScan();
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    return CHIP_NO_ERROR;
}

int ChipDeviceScanner::MainLoopStopScan(ChipDeviceScanner * self)
{
    return 0;
}

int ChipDeviceScanner::MainLoopStartScan(ChipDeviceScanner * self)
{
    return 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
