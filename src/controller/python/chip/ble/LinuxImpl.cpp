
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/bluez/AdapterIterator.h>
#include <platform/Linux/bluez/MainLoop.h>
#include <platform/internal/BLEManager.h>

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

// To avoid pythoon compatibility issues on inc/dec references,
// code assumes an abstract type and leaves it up to python to keep track of
// reference counts
struct PyObject;

class ScannerDelegateImpl : public ChipDeviceScannerDelegate
{
public:
    using DeviceScannedCallback = void (*)(PyObject * context, const char * address, uint16_t discriminator, uint16_t vendorId,
                                           uint16_t productId);
    using ScanCompleteCallback  = void (*)(PyObject * context);

    ScannerDelegateImpl(PyObject * context, DeviceScannedCallback scanCallback, ScanCompleteCallback completeCallback) :
        mContext(context), mScanCallback(scanCallback), mCompleteCallback(completeCallback)
    {}

    void SetScanner(std::unique_ptr<ChipDeviceScanner> scanner) { mScanner = std::move(scanner); }

    void OnDeviceScanned(BluezDevice1 * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override
    {
        if (mScanCallback)
        {
            mScanCallback(mContext, bluez_device1_get_address(device), info.GetDeviceDiscriminator(), info.GetProductId(),
                          info.GetVendorId());
        }
    }

    void OnScanComplete() override
    {
        if (mCompleteCallback)
        {
            mCompleteCallback(mContext);
        }

        delete this;
    }

private:
    std::unique_ptr<ChipDeviceScanner> mScanner;
    PyObject * const mContext;
    const DeviceScannedCallback mScanCallback;
    const ScanCompleteCallback mCompleteCallback;
};

} // namespace

extern "C" void * pychip_ble_start_scanning(PyObject * context, void * adapter, uint32_t timeoutMs,
                                            ScannerDelegateImpl::DeviceScannedCallback scanCallback,
                                            ScannerDelegateImpl::ScanCompleteCallback completeCallback)
{
    std::unique_ptr<ScannerDelegateImpl> delegate = std::make_unique<ScannerDelegateImpl>(context, scanCallback, completeCallback);

    std::unique_ptr<ChipDeviceScanner> scanner = ChipDeviceScanner::Create(static_cast<BluezAdapter1 *>(adapter), delegate.get());

    if (!scanner)
    {
        return nullptr;
    }

    if (scanner->StartScan(chip::System::Clock::Milliseconds32(timeoutMs)) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    delegate->SetScanner(std::move(scanner));

    return delegate.release();
}
