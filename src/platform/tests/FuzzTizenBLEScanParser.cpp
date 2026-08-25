#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
#include <bluetooth.h>
}

#include <lib/support/CodeUtils.h>

namespace {

constexpr char kRemoteAddress[]     = "00:11:22:33:44:55";
constexpr uint8_t kMatterUuidLow    = 0xF6;
constexpr uint8_t kMatterUuidHigh   = 0xFF;
constexpr uint8_t kDecoyUuidLow     = 0x34;
constexpr uint8_t kDecoyUuidHigh    = 0x12;
constexpr size_t kMaxServiceDataLen = 124;

bt_adapter_le_device_scan_result_info_s * gStartScanResult = nullptr;
int gStartScanReturn                                       = BT_ERROR_NONE;
int gStartScanCallbackResult                               = BT_ERROR_NONE;
int gScanFilterReturn                                      = BT_ERROR_NONE;
bool gScanFilterSupported                                  = true;
bool gInvokeStartScanCallback                              = false;

void AppendServiceData(std::vector<char> & output, bool matterUuid, const uint8_t * data, size_t dataLen)
{
    const size_t boundedLen = std::min(dataLen, kMaxServiceDataLen);
    output.push_back(static_cast<char>(boundedLen + 3));
    output.push_back(static_cast<char>(BT_ADAPTER_LE_ADVERTISING_DATA_SERVICE_DATA));
    output.push_back(static_cast<char>(matterUuid ? kMatterUuidLow : kDecoyUuidLow));
    output.push_back(static_cast<char>(matterUuid ? kMatterUuidHigh : kDecoyUuidHigh));

    for (size_t i = 0; i < boundedLen; ++i)
    {
        output.push_back(static_cast<char>(data[i]));
    }
}

std::vector<char> BuildPacket(const uint8_t * data, size_t len, bool forceMatterUuid, bool includeDecoy)
{
    std::vector<char> packet;
    const size_t decoyLen = includeDecoy ? len / 2 : 0;

    if (includeDecoy)
    {
        AppendServiceData(packet, false, data, decoyLen);
    }
    AppendServiceData(packet, forceMatterUuid, data + decoyLen, len - decoyLen);

    return packet;
}

} // namespace

extern "C" int bt_adapter_le_start_scan(void (*callback)(int, bt_adapter_le_device_scan_result_info_s *, void *), void * userData)
{
    if (gInvokeStartScanCallback && callback != nullptr)
    {
        callback(gStartScanCallbackResult, gStartScanResult, userData);
    }
    return gStartScanReturn;
}

extern "C" int bt_adapter_le_stop_scan(void)
{
    return BT_ERROR_NONE;
}

extern "C" int bt_adapter_le_is_scan_filter_supported(bool * isSupported)
{
    if (isSupported == nullptr)
    {
        return BT_ERROR_INVALID_PARAMETER;
    }

    *isSupported = gScanFilterSupported;
    return BT_ERROR_NONE;
}

extern "C" int bt_adapter_le_scan_filter_create(bt_scan_filter_h * filter)
{
    if (filter == nullptr)
    {
        return BT_ERROR_INVALID_PARAMETER;
    }

    *filter = reinterpret_cast<bt_scan_filter_h>(0x1);
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_unregister(bt_scan_filter_h)
{
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_destroy(bt_scan_filter_h)
{
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_set_device_address(bt_scan_filter_h, const char *)
{
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_set_service_uuid(bt_scan_filter_h, const char *)
{
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_set_service_data(bt_scan_filter_h, const char *, const char *, unsigned int)
{
    return gScanFilterReturn;
}

extern "C" int bt_adapter_le_scan_filter_register(bt_scan_filter_h)
{
    return gScanFilterReturn;
}

extern "C" const char * get_error_message(int)
{
    return "stub";
}

#define private public
#include <platform/Tizen/ChipDeviceScanner.cpp>
#undef private

namespace {

class RecordingDelegate : public chip::DeviceLayer::Internal::ChipDeviceScannerDelegate
{
public:
    void OnDeviceScanned(const bt_adapter_le_device_scan_result_info_s &,
                         const chip::Ble::ChipBLEDeviceIdentificationInfo &) override
    {
        deviceScannedCount++;
    }

    void OnScanComplete() override { scanCompleteCount++; }
    void OnScanError(CHIP_ERROR) override { scanErrorCount++; }

    size_t deviceScannedCount = 0;
    size_t scanCompleteCount  = 0;
    size_t scanErrorCount     = 0;
};

chip::DeviceLayer::Internal::ScanFilterType PickFilterType(uint8_t value)
{
    using chip::DeviceLayer::Internal::ScanFilterType;

    switch (value & 0x03)
    {
    case 0:
        return ScanFilterType::kAddress;
    case 1:
        return ScanFilterType::kServiceUUID;
    case 2:
        return ScanFilterType::kServiceData;
    default:
        return ScanFilterType::kNoFilter;
    }
}

void FillFilterData(chip::DeviceLayer::Internal::ScanFilterData & filterData, const uint8_t * data, size_t len)
{
    std::memset(&filterData, 0, sizeof(filterData));

    constexpr char fallbackAddress[] = "AA:BB:CC:DD:EE:FF";
    constexpr char fallbackUuid[]    = "0000fff6-0000-1000-8000-00805f9b34fb";
    std::memcpy(filterData.address, fallbackAddress, sizeof(fallbackAddress));
    std::memcpy(filterData.service_uuid, fallbackUuid, sizeof(fallbackUuid));

    const size_t serviceDataLen = std::min(sizeof(filterData.service_data), len);
    if (serviceDataLen > 0)
    {
        std::memcpy(filterData.service_data, data, serviceDataLen);
    }
    filterData.service_data_len = static_cast<unsigned int>(serviceDataLen);
}

void ExerciseTizenRTParser(bt_adapter_le_device_scan_result_info_s & scanInfo, uint8_t selector)
{
    bt_adapter_le_service_data_s * dataList = nullptr;
    int count                               = 0;
    const auto packetType = (selector & 0x01) ? BT_ADAPTER_LE_PACKET_SCAN_RESPONSE : BT_ADAPTER_LE_PACKET_ADVERTISING;

    if (bt_adapter_le_get_scan_result_service_data_list(&scanInfo, packetType, &dataList, &count) == BT_ERROR_NONE)
    {
        bt_adapter_le_free_service_data_list(dataList, count);
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    if (len < 4)
    {
        return 0;
    }

    const uint8_t flags     = data[0];
    const size_t split      = 1 + (data[1] % (len - 1));
    const bool matterUuid   = (flags & 0x01) != 0;
    const bool includeDecoy = (flags & 0x02) != 0;
    const bool scannerError = (flags & 0x04) != 0;
    const bool nullScanInfo = (flags & 0x08) != 0;

    std::vector<char> advData  = BuildPacket(data + 2, split - 1, matterUuid, includeDecoy);
    std::vector<char> scanData = BuildPacket(data + split, len - split, (flags & 0x10) != 0, (flags & 0x20) != 0);

    bt_adapter_le_device_scan_result_info_s scanInfo = {
        kRemoteAddress,
        advData.empty() ? nullptr : advData.data(),
        static_cast<int>(advData.size()),
        scanData.empty() ? nullptr : scanData.data(),
        static_cast<int>(scanData.size()),
    };

    RecordingDelegate delegate;
    chip::DeviceLayer::Internal::ChipDeviceScanner scanner(&delegate);

    ExerciseTizenRTParser(scanInfo, flags >> 6);

    scanner.LeScanResultCb(scannerError ? BT_ERROR_INVALID_PARAMETER : BT_ERROR_NONE, nullScanInfo ? nullptr : &scanInfo);

    chip::DeviceLayer::Internal::ScanFilterData filterData;
    FillFilterData(filterData, data + 2, len - 2);

    gStartScanResult         = &scanInfo;
    gStartScanCallbackResult = scannerError ? BT_ERROR_INVALID_PARAMETER : BT_ERROR_NONE;
    gStartScanReturn         = (flags & 0x40) ? BT_ERROR_INVALID_PARAMETER : BT_ERROR_NONE;
    gScanFilterReturn        = (flags & 0x80) ? BT_ERROR_INVALID_PARAMETER : BT_ERROR_NONE;
    gScanFilterSupported     = (data[2] & 0x01) == 0;
    gInvokeStartScanCallback = (data[2] & 0x02) != 0;

    (void) scanner.SetupScanFilter(PickFilterType(data[3]), filterData);
    (void) scanner.RegisterScanFilter(PickFilterType(data[3] >> 2), filterData);
    (void) scanner.StartScan(PickFilterType(data[3] >> 4), filterData);
    (void) scanner.StopScan();

    gStartScanResult = nullptr;
    return 0;
}
