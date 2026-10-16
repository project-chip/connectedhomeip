/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include <cstdint>
#include <memory>

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/BlePlatformConfig.h>
#include <platform/Linux/bluez/AdapterIterator.h>
#include <platform/Linux/bluez/BluezObjectManager.h>
#include <platform/Linux/bluez/ChipDeviceScanner.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

using namespace chip::DeviceLayer::Internal;

/////////// Listing adapters implementation //////////

extern "C" void * pychip_ble_adapter_list_new()
{
    return static_cast<void *>(new AdapterIterator());
}

extern "C" void pychip_ble_adapter_list_delete(void * adapterIterator)
{
    delete static_cast<AdapterIterator *>(adapterIterator);
}

extern "C" bool pychip_ble_adapter_list_next(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->Next();
}

extern "C" uint32_t pychip_ble_adapter_list_get_index(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->GetIndex();
}

extern "C" const char * pychip_ble_adapter_list_get_address(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->GetAddress();
}

extern "C" const char * pychip_ble_adapter_list_get_alias(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->GetAlias();
}

extern "C" const char * pychip_ble_adapter_list_get_name(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->GetName();
}

extern "C" bool pychip_ble_adapter_list_is_powered(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->IsPowered();
}

extern "C" void * pychip_ble_adapter_list_get_raw_adapter(void * adapterIterator)
{
    return static_cast<AdapterIterator *>(adapterIterator)->GetAdapter();
}

/////////// CHIP Device scanner implementation //////////

namespace {

// To avoid python compatibility issues on inc/dec references,
// code assumes an abstract type and leaves it up to python to keep track of
// reference counts
struct PyObject;

class ScannerDelegateImpl : public ChipDeviceScannerDelegate
{
public:
    using DeviceScannedCallback = void (*)(PyObject * context, const char * address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId);
    using ScanCompleteCallback  = void (*)(PyObject * context);
    using ScanErrorCallback     = void (*)(PyObject * context, CHIP_ERROR::StorageType error);

    ScannerDelegateImpl(PyObject * context, DeviceScannedCallback scanCallback, ScanCompleteCallback completeCallback,
                        ScanErrorCallback errorCallback) :
        mContext(context),
        mScanCallback(scanCallback), mCompleteCallback(completeCallback), mErrorCallback(errorCallback)
    {}

    ~ScannerDelegateImpl() override
    {
        mScanner.Shutdown();
        mBluezObjectManager.Shutdown();
    }

    CHIP_ERROR ScannerInit(BluezAdapter1 * adapter)
    {
        ReturnErrorOnFailure(mBluezObjectManager.Init());
        return mScanner.Init(adapter, this);
    }

    CHIP_ERROR ScannerStartScan(chip::System::Clock::Timeout timeout)
    {
        mDeferringCompletion = true;
        CHIP_ERROR err       = mScanner.StartScan();
        if (err == CHIP_NO_ERROR)
        {
            err = chip::DeviceLayer::SystemLayer().StartTimer(timeout, HandleScannerTimer, this);
        }
        if (err != CHIP_NO_ERROR)
        {
            RETURN_SAFELY_IGNORED mScanner.StopScan();
        }
        mDeferringCompletion = false;
        if (err != CHIP_NO_ERROR)
        {
            OnScanError(err);
        }
        return err;
    }

    CHIP_ERROR ScannerStopScan()
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(HandleScannerTimer, this);
        mDeferringCompletion = true;
        CHIP_ERROR err       = mScanner.StopScan();
        mDeferringCompletion = false;
        if (err != CHIP_NO_ERROR)
        {
            OnScanError(err);
        }
        else
        {
            OnScanComplete();
        }
        return err;
    }

    static void HandleScannerTimer(chip::System::Layer *, void * appState)
    {
        auto * delegate = static_cast<ScannerDelegateImpl *>(appState);
        RETURN_SAFELY_IGNORED delegate->ScannerStopScan();
    }

    void OnDeviceScanned(BluezDevice1 & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override
    {
        if (mScanCallback)
        {
            mScanCallback(mContext, bluez_device1_get_address(&device), info.GetDeviceDiscriminator(), info.GetProductId(),
                          info.GetVendorId());
        }
    }

    void OnScanComplete() override
    {
        VerifyOrReturn(!mDeferringCompletion && !mTerminalReported);
        mTerminalReported = true;
        if (mCompleteCallback)
        {
            mCompleteCallback(mContext);
        }
    }

    void OnScanError(CHIP_ERROR error) override
    {
        VerifyOrReturn(!mTerminalReported);
        mTerminalReported = true;
        if (mErrorCallback)
        {
            mErrorCallback(mContext, error.AsInteger());
        }
    }

private:
    BluezObjectManager mBluezObjectManager;
    ChipDeviceScanner mScanner{ mBluezObjectManager };
    PyObject * const mContext;
    const DeviceScannedCallback mScanCallback;
    const ScanCompleteCallback mCompleteCallback;
    const ScanErrorCallback mErrorCallback;
    bool mDeferringCompletion = false;
    bool mTerminalReported    = false;
};

} // namespace

extern "C" void * pychip_ble_scanner_start(PyObject * context, void * adapter, uint32_t timeoutMs,
                                           ScannerDelegateImpl::DeviceScannedCallback scanCallback,
                                           ScannerDelegateImpl::ScanCompleteCallback completeCallback,
                                           ScannerDelegateImpl::ScanErrorCallback errorCallback)
{
    std::unique_ptr<ScannerDelegateImpl> delegate =
        std::make_unique<ScannerDelegateImpl>(context, scanCallback, completeCallback, errorCallback);

    CHIP_ERROR err = delegate->ScannerInit(static_cast<BluezAdapter1 *>(adapter));
    chip::DeviceLayer::StackLock lock;
    if (err == CHIP_NO_ERROR)
    {
        err = delegate->ScannerStartScan(chip::System::Clock::Milliseconds32(timeoutMs));
    }
    else
    {
        delegate->OnScanError(err);
    }
    if (err != CHIP_NO_ERROR)
    {
        delegate.reset();
        return nullptr;
    }

    return delegate.release();
}

extern "C" void pychip_ble_scanner_delete(void * scanner)
{
    auto * delegate = static_cast<ScannerDelegateImpl *>(scanner);
    chip::DeviceLayer::StackLock lock;
    // Make sure that the scanner is stopped before deleting the delegate.
    RETURN_SAFELY_IGNORED delegate->ScannerStopScan();
    delete delegate;
}
