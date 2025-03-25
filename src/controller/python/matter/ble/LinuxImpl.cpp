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

    CHIP_ERROR ScannerInit(BluezAdapter1 * adapter)
    {
        ReturnErrorOnFailure(mBluezObjectManager.Init());
        return mScanner.Init(adapter, this);
    }

    void ScannerShutdown() { mBluezObjectManager.Shutdown(); }

    CHIP_ERROR ScannerStartScan(chip::System::Clock::Timeout timeout)
    {
        CHIP_ERROR err = mScanner.StartScan();
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        err = chip::DeviceLayer::SystemLayer().StartTimer(timeout, HandleScannerTimer, this);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err, mScanner.StopScan());

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ScannerStopScan()
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(HandleScannerTimer, this);
        return mScanner.StopScan();
    }

    static void HandleScannerTimer(chip::System::Layer *, void * appState)
    {
        auto * delegate = static_cast<ScannerDelegateImpl *>(appState);
        delegate->OnScanError(CHIP_ERROR_TIMEOUT);
        delegate->mScanner.StopScan();
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
        if (mCompleteCallback)
        {
            mCompleteCallback(mContext);
        }
    }

    void OnScanError(CHIP_ERROR error) override
    {
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
    VerifyOrReturnError(err == CHIP_NO_ERROR, nullptr);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    err = delegate->ScannerStartScan(chip::System::Clock::Milliseconds32(timeoutMs));
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    VerifyOrReturnError(err == CHIP_NO_ERROR, nullptr);

    return delegate.release();
}

extern "C" void pychip_ble_scanner_delete(void * scanner)
{
    auto * delegate = static_cast<ScannerDelegateImpl *>(scanner);
    chip::DeviceLayer::StackLock lock;
    // Make sure that the scanner is stopped before deleting the delegate.
    delegate->ScannerStopScan();
    delegate->ScannerShutdown();
    delete delegate;
}
