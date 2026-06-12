/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#define private public
#include <platform/webos/wbs/WbsDeviceScanner.h>
#undef private

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace {

constexpr char kMatterServiceUuid[] = "fff6";
constexpr char kRemoteAddress[]     = "00:11:22:33:44:55";
constexpr size_t kMaxDirectBytes    = 128;
constexpr size_t kMaxScanRecordData = 128;

enum class ServiceUuidChoice : uint8_t
{
    kMatter,
    kNull,
    kAllSet,
};

std::string BytesToHex(const uint8_t * data, size_t len)
{
    constexpr char kHex[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(len * 2);

    for (size_t i = 0; i < len; ++i)
    {
        hex.push_back(kHex[data[i] >> 4]);
        hex.push_back(kHex[data[i] & 0x0f]);
    }

    return hex;
}

pbnjson::JValue MakeMatterServiceUuidArray()
{
    pbnjson::JValue serviceUuid = pbnjson::JArray();
    serviceUuid.append(kMatterServiceUuid);
    return serviceUuid;
}

pbnjson::JValue MakeScanRecordArray(const std::vector<uint8_t> & scanRecord)
{
    pbnjson::JValue scanRecordArray = pbnjson::JArray();
    for (uint8_t byte : scanRecord)
    {
        scanRecordArray.append(static_cast<int32_t>(byte));
    }
    return scanRecordArray;
}

ServiceUuidChoice PickServiceUuid(uint8_t selector)
{
    switch (selector % 3)
    {
    case 0:
        return ServiceUuidChoice::kMatter;
    case 1:
        return ServiceUuidChoice::kNull;
    default:
        return ServiceUuidChoice::kAllSet;
    }
}

void AppendServiceUuid(std::vector<uint8_t> & scanRecord, ServiceUuidChoice choice)
{
    switch (choice)
    {
    case ServiceUuidChoice::kMatter:
        scanRecord.push_back(0xf6);
        scanRecord.push_back(0xff);
        break;
    case ServiceUuidChoice::kNull:
        scanRecord.push_back(0x00);
        scanRecord.push_back(0x00);
        break;
    case ServiceUuidChoice::kAllSet:
        scanRecord.push_back(0xff);
        scanRecord.push_back(0xff);
        break;
    }
}

void AppendBoundedAdStructure(std::vector<uint8_t> & scanRecord, uint8_t fuzzedLength, uint8_t fuzzedType,
                              ServiceUuidChoice uuidChoice, const uint8_t * data, size_t len)
{
    if (scanRecord.size() >= kMaxScanRecordData)
    {
        return;
    }

    const size_t payloadCapacity = std::min<size_t>(fuzzedLength, kMaxScanRecordData - scanRecord.size() - 1);
    if (payloadCapacity == 0)
    {
        return;
    }

    scanRecord.push_back(static_cast<uint8_t>(payloadCapacity));
    scanRecord.push_back(fuzzedType);

    size_t payloadBytesWritten = 1;
    if (payloadCapacity >= 3)
    {
        AppendServiceUuid(scanRecord, uuidChoice);
        payloadBytesWritten += 2;
    }

    const size_t fuzzPayloadLen = std::min(len, payloadCapacity - payloadBytesWritten);
    if (fuzzPayloadLen > 0)
    {
        std::copy(data, data + fuzzPayloadLen, std::back_inserter(scanRecord));
    }

    for (size_t i = fuzzPayloadLen; i < payloadCapacity - payloadBytesWritten; ++i)
    {
        scanRecord.push_back(static_cast<uint8_t>(i));
    }
}

std::vector<uint8_t> MakeFuzzedScanRecord(const uint8_t * data, size_t len)
{
    std::vector<uint8_t> scanRecord;
    scanRecord.reserve(kMaxScanRecordData);

    if (len < 6)
    {
        return scanRecord;
    }

    const ServiceUuidChoice serviceListUuid = PickServiceUuid(data[0]);
    const ServiceUuidChoice serviceDataUuid = PickServiceUuid(data[1]);
    const uint8_t serviceListLength         = static_cast<uint8_t>(1 + (data[2] % 8));
    const uint8_t serviceListType           = data[3];
    const uint8_t serviceDataLength         = static_cast<uint8_t>(1 + (data[4] % 64));
    const uint8_t serviceDataType           = data[5];

    AppendBoundedAdStructure(scanRecord, serviceListLength, serviceListType, serviceListUuid, data + 6, len - 6);
    AppendBoundedAdStructure(scanRecord, serviceDataLength, serviceDataType, serviceDataUuid, data + 6, len - 6);

    return scanRecord;
}

pbnjson::JValue MakeDirectServiceDataDevice(const uint8_t * data, size_t len)
{
    constexpr size_t kOverflowBytes = sizeof(chip::Ble::ChipBLEDeviceIdentificationInfo) + 32;

    std::array<uint8_t, kOverflowBytes + kMaxDirectBytes> serviceData = {};
    for (size_t i = 0; i < kOverflowBytes; ++i)
    {
        serviceData[i] = static_cast<uint8_t>(i);
    }

    const size_t fuzzLen = std::min(len, kMaxDirectBytes);
    if (fuzzLen > 0)
    {
        std::copy(data, data + fuzzLen, serviceData.begin() + kOverflowBytes);
    }

    pbnjson::JValue device = pbnjson::JObject();
    device.put("address", kRemoteAddress);
    device.put("serviceUuid", MakeMatterServiceUuidArray());
    device.put("serviceData", BytesToHex(serviceData.data(), kOverflowBytes + fuzzLen));
    device.put("scanRecord", pbnjson::JArray());

    return device;
}

pbnjson::JValue MakeScanRecordDevice(const uint8_t * data, size_t len)
{
    pbnjson::JValue device = pbnjson::JObject();
    device.put("address", kRemoteAddress);
    device.put("scanRecord", MakeScanRecordArray(MakeFuzzedScanRecord(data, len)));

    return device;
}

class RecordingDelegate : public chip::DeviceLayer::Internal::WbsDeviceScannerDelegate
{
public:
    void OnDeviceScanned(const pbnjson::JValue &, const chip::Ble::ChipBLEDeviceIdentificationInfo &) override
    {
        deviceScannedCount++;
    }

    void OnScanComplete() override { scanCompleteCount++; }
    void OnScanError(CHIP_ERROR) override { scanErrorCount++; }

    size_t deviceScannedCount = 0;
    size_t scanCompleteCount  = 0;
    size_t scanErrorCount     = 0;
};

void EnsureChipMemoryInitialized()
{
    static const bool initialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) initialized;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    EnsureChipMemoryInitialized();

    RecordingDelegate delegate;
    chip::DeviceLayer::Internal::WbsDeviceScanner scanner;
    scanner.mDelegate = &delegate;

    scanner.ReportDevice(MakeDirectServiceDataDevice(data, len));
    scanner.ReportDevice(MakeScanRecordDevice(data, len));

    return 0;
}
