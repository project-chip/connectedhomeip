#include <ble/Ble.h>
#include <lib/support/SafeInt.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <luna-service2++/handle.hpp>
#include "lsrequester.h"
#include "Helper.h"

#include <chrono>
#include <thread>
#include <vector>
#include <string>

// [LGE_MATTER_COMPAT_PATCH]
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

#define API_BLUETOOTH_GATT_GETSTATUS "luna://com.webos.service.bluetooth2/gatt/getStatus"
#define API_BLUETOOTH_GATT_CONNECT "luna://com.webos.service.bluetooth2/gatt/connect"
#define API_BLUETOOTH_GATT_DISCONNECT "luna://com.webos.service.bluetooth2/gatt/disconnect"
#define API_BLUETOOTH_GATT_DISCOVERSERVICES "luna://com.webos.service.bluetooth2/gatt/discoverServices"
#define API_BLUETOOTH_GATT_GETSERVICES "luna://com.webos.service.bluetooth2/gatt/getServices"

#define API_BLUETOOTH_GATT_MONITORCHRACTERISTICS "luna://com.webos.service.bluetooth2/gatt/monitorCharacteristics"
#define API_BLUETOOTH_GATT_WRITEDESCRIPTOR "luna://com.webos.service.bluetooth2/gatt/writeDescriptorValue"
#define API_BLUETOOTH_GATT_WRITECHRACTERISTIC "luna://com.webos.service.bluetooth2/gatt/writeCharacteristicValue"
#define API_BLUETOOTH_GATT_REQUESTMTU "luna://com.webos.service.bluetooth2/gatt/requestMtu"

#define CHIP_BLE_GATT_SERVICE "0000fff6-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_GATT_CHAR_WRITE "18ee2ef5-263d-4559-959f-4f9c429f9d11"
#define CHIP_BLE_GATT_CHAR_READ "18ee2ef5-263d-4559-959f-4f9c429f9d12"

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kMaxConnectRetries = 4;

// Workaround patch for meross smart plug: set when the peer TX characteristic supports notify only.
static bool isNotSupportIndication = false;

// [LGE_MATTER_COMPAT_PATCH]
#define HOMEYMATTER_WHITELIST_PATH "/mnt/lg/cmn_data/.homeymatter/homeymatter_whitelist.json"

// Whitelist type identifiers
#define WHITELIST_TYPE_BLE_PACING "blePacing"

// BLE Pacing defaults
#define BLE_PACING_DEFAULT_DELAY_MS 100
#define BLE_PACING_FAILURE_THRESHOLD 2  // consecutive TIMEOUT failures (stage 10-18) to add
#define BLE_PACING_REMOVAL_THRESHOLD 2  // consecutive ANY failures to remove (when already in whitelist)

static pbnjson::JValue sWhitelistRoot;
static bool sWhitelistFileLoaded = false;

// Load the entire whitelist file (file should already exist, created by WhitelistManager)
static void LoadWhitelistFile()
{
    if (sWhitelistFileLoaded)
        return;
    sWhitelistFileLoaded = true;

    // [LGE_MATTER_COMPAT_PATCH] O_NOFOLLOW: whitelist path is world-writable
    int fd = open(HOMEYMATTER_WHITELIST_PATH, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    FILE * f = (fd >= 0) ? fdopen(fd, "r") : nullptr;
    if (f == nullptr)
    {
        if (fd >= 0)
            close(fd);
        ChipLogError(DeviceLayer, "Whitelist file open(r) failed: %s (errno=%d)", HOMEYMATTER_WHITELIST_PATH, errno);
        sWhitelistRoot = pbnjson::JObject();
        return;
    }

    // Read existing file
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 64 * 1024)
    {
        fclose(f);
        ChipLogError(DeviceLayer, "Whitelist file: invalid file size %ld", fsize);
        sWhitelistRoot = pbnjson::JObject();
        return;
    }

    std::string content(static_cast<size_t>(fsize), '\0');
    size_t readBytes = fread(&content[0], 1, static_cast<size_t>(fsize), f);
    fclose(f);
    content.resize(readBytes);

    sWhitelistRoot = pbnjson::JDomParser::fromString(content);
    if (!sWhitelistRoot.isValid())
    {
        ChipLogError(DeviceLayer, "Whitelist file: invalid JSON");
        sWhitelistRoot = pbnjson::JObject();
        return;
    }

    ChipLogProgress(DeviceLayer, "Whitelist file loaded: %s", HOMEYMATTER_WHITELIST_PATH);
}

// Save the entire whitelist file with pretty formatting
static void SaveWhitelistFile()
{
    // [LGE_MATTER_COMPAT_PATCH] O_NOFOLLOW: whitelist path is world-writable
    int fd = open(HOMEYMATTER_WHITELIST_PATH, O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW | O_CLOEXEC, 0644);
    FILE * f = (fd >= 0) ? fdopen(fd, "w") : nullptr;
    if (f == nullptr)
    {
        if (fd >= 0)
            close(fd);
        ChipLogError(DeviceLayer, "Whitelist file: failed to open for write (errno=%d)", errno);
        return;
    }

    // Get version from root, default to 1
    int version = 1;
    if (sWhitelistRoot.hasKey("version"))
    {
        version = sWhitelistRoot["version"].asNumber<int32_t>();
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"version\": %d,\n\n", version);

    // Collect all type keys (skip "version")
    std::vector<std::string> typeKeys;
    for (auto it = sWhitelistRoot.begin(); it != sWhitelistRoot.end(); ++it)
    {
        std::string key = (*it).first.asString();
        if (key != "version")
        {
            typeKeys.push_back(key);
        }
    }

    // Write each type section
    for (size_t t = 0; t < typeKeys.size(); ++t)
    {
        const std::string & typeName = typeKeys[t];
        pbnjson::JValue section = sWhitelistRoot[typeName];

        fprintf(f, "  \"%s\": {\n", typeName.c_str());

        if (section.hasKey("description"))
        {
            fprintf(f, "    \"description\": \"%s\",\n", section["description"].asString().c_str());
        }

        fprintf(f, "    \"devices\": [\n");
        if (section.hasKey("devices"))
        {
            pbnjson::JValue devices = section["devices"];
            for (int i = 0; i < devices.arraySize(); ++i)
            {
                pbnjson::JValue entry = devices[i];
                fprintf(f, "      {");

                // Write entry fields
                bool firstField = true;
                if (entry.hasKey("vendorId"))
                {
                    fprintf(f, "\"vendorId\": \"%s\"", entry["vendorId"].asString().c_str());
                    firstField = false;
                }
                if (entry.hasKey("productId"))
                {
                    fprintf(f, "%s\"productId\": \"%s\"", firstField ? "" : ", ", entry["productId"].asString().c_str());
                    firstField = false;
                }
                if (entry.hasKey("delayMs"))
                {
                    fprintf(f, "%s\"delayMs\": %d", firstField ? "" : ", ", entry["delayMs"].asNumber<int32_t>());
                    firstField = false;
                }
                if (entry.hasKey("name"))
                {
                    fprintf(f, "%s\"name\": \"%s\"", firstField ? "" : ", ", entry["name"].asString().c_str());
                }

                fprintf(f, "}%s\n", (i < devices.arraySize() - 1) ? "," : "");
            }
        }
        fprintf(f, "    ]\n");
        fprintf(f, "  }%s\n", (t < typeKeys.size() - 1) ? ",\n" : "");
    }

    fprintf(f, "}\n");
    fclose(f);
    ChipLogDetail(DeviceLayer, "Whitelist file saved");
}

// Get devices array for a specific whitelist type
static pbnjson::JValue GetWhitelistDevices(const char * typeName)
{
    LoadWhitelistFile();
    if (sWhitelistRoot.hasKey(typeName))
    {
        pbnjson::JValue section = sWhitelistRoot[typeName];
        if (section.hasKey("devices"))
        {
            return section["devices"];
        }
    }
    return pbnjson::JArray();
}

// [LGE_MATTER_COMPAT_PATCH]
// ============================================================================
// BLE Write Pacing Whitelist (uses generic whitelist infrastructure)
// ============================================================================

struct BlePacingEntry
{
    uint16_t vendorId;
    uint16_t productId;
    int delayMs;
};

static std::vector<BlePacingEntry> sPacingWhitelist;
static bool sPacingWhitelistParsed = false;
// Written on the Matter thread, read on the Matter GLib context (SendWriteRequestImpl).
static std::atomic<int> sActivePacingDelayMs{ 0 }; // 0 = pacing disabled for current connection

// Parse blePacing section into in-memory cache
static void ParsePacingWhitelist()
{
    if (sPacingWhitelistParsed)
        return;
    sPacingWhitelistParsed = true;
    sPacingWhitelist.clear();

    pbnjson::JValue devices = GetWhitelistDevices(WHITELIST_TYPE_BLE_PACING);
    for (int i = 0; i < devices.arraySize(); ++i)
    {
        pbnjson::JValue entry = devices[i];
        if (!entry.hasKey("vendorId") || !entry.hasKey("productId"))
            continue;

        uint16_t vid = 0, pid = 0;
        int delay = BLE_PACING_DEFAULT_DELAY_MS;

        std::string vidStr = entry["vendorId"].asString();
        std::string pidStr = entry["productId"].asString();
        vid = static_cast<uint16_t>(strtoul(vidStr.c_str(), nullptr, 0));
        pid = static_cast<uint16_t>(strtoul(pidStr.c_str(), nullptr, 0));

        if (entry.hasKey("delayMs"))
            delay = entry["delayMs"].asNumber<int32_t>();

        if (vid != 0 && pid != 0)
        {
            sPacingWhitelist.push_back({vid, pid, delay});
        }
    }

    ChipLogProgress(DeviceLayer, "BLE pacing whitelist parsed: %zu devices", sPacingWhitelist.size());
    for (const auto & e : sPacingWhitelist)
    {
        ChipLogDetail(DeviceLayer, "  vid=0x%04X pid=0x%04X delay=%dms", e.vendorId, e.productId, e.delayMs);
    }
}

static int GetPacingDelayForDevice(uint16_t vendorId, uint16_t productId)
{
    ParsePacingWhitelist();
    for (const auto & entry : sPacingWhitelist)
    {
        if (entry.vendorId == vendorId && entry.productId == productId)
            return entry.delayMs;
    }
    return 0; // not in whitelist
}

void SetActivePacingDelay(uint16_t vendorId, uint16_t productId)
{
    const int delayMs = GetPacingDelayForDevice(vendorId, productId);
    sActivePacingDelayMs.store(delayMs);
    if (delayMs > 0)
    {
        ChipLogProgress(DeviceLayer, "BLE write pacing ENABLED for vid=0x%04X pid=0x%04X: %dms per write",
                        vendorId, productId, delayMs);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "BLE write pacing disabled for vid=0x%04X pid=0x%04X (not in whitelist)",
                        vendorId, productId);
    }
}

void ClearActivePacingDelay()
{
    sActivePacingDelayMs = 0;
}

// Sync in-memory cache back to JSON and save file
static void SyncPacingWhitelistToFile()
{
    LoadWhitelistFile();

    // Rebuild devices array from in-memory cache
    pbnjson::JValue devices = pbnjson::JArray();
    for (const auto & e : sPacingWhitelist)
    {
        pbnjson::JValue entry = pbnjson::JObject();
        char vidStr[16], pidStr[16];
        snprintf(vidStr, sizeof(vidStr), "0x%04X", e.vendorId);
        snprintf(pidStr, sizeof(pidStr), "0x%04X", e.productId);
        entry.put("vendorId", vidStr);
        entry.put("productId", pidStr);
        entry.put("delayMs", e.delayMs);
        devices.append(entry);
    }

    // Update section
    if (!sWhitelistRoot.hasKey(WHITELIST_TYPE_BLE_PACING))
    {
        // Section should already exist (created by WhitelistManager), but create if missing
        pbnjson::JValue section = pbnjson::JObject();
        section.put("description", "BLE write pacing - devices that need delay between BLE writes during commissioning");
        section.put("devices", devices);
        sWhitelistRoot.put(WHITELIST_TYPE_BLE_PACING, section);
    }
    else
    {
        sWhitelistRoot[WHITELIST_TYPE_BLE_PACING].put("devices", devices);
    }

    SaveWhitelistFile();
}

static void AddToPacingWhitelist(uint16_t vendorId, uint16_t productId)
{
    ParsePacingWhitelist();

    // Check if already exists
    for (const auto & entry : sPacingWhitelist)
    {
        if (entry.vendorId == vendorId && entry.productId == productId)
            return;
    }

    sPacingWhitelist.push_back({vendorId, productId, BLE_PACING_DEFAULT_DELAY_MS});
    SyncPacingWhitelistToFile();

    ChipLogProgress(DeviceLayer, "BLE pacing whitelist: auto-added vid=0x%04X pid=0x%04X delay=%dms",
                    vendorId, productId, BLE_PACING_DEFAULT_DELAY_MS);
}

static void RemoveFromPacingWhitelist(uint16_t vendorId, uint16_t productId)
{
    ParsePacingWhitelist();

    auto it = sPacingWhitelist.begin();
    while (it != sPacingWhitelist.end())
    {
        if (it->vendorId == vendorId && it->productId == productId)
        {
            it = sPacingWhitelist.erase(it);
            SyncPacingWhitelistToFile();
            ChipLogProgress(DeviceLayer, "BLE pacing whitelist: auto-removed vid=0x%04X pid=0x%04X",
                            vendorId, productId);
            return;
        }
        ++it;
    }
}

static bool IsInPacingWhitelist(uint16_t vendorId, uint16_t productId)
{
    ParsePacingWhitelist();
    for (const auto & entry : sPacingWhitelist)
    {
        if (entry.vendorId == vendorId && entry.productId == productId)
            return true;
    }
    return false;
}

// [LGE_MATTER_COMPAT_PATCH]
// ============================================================================
// Auto-detection: Failure tracking and whitelist toggle
// ============================================================================

struct PacingFailureRecord
{
    uint16_t vendorId;
    uint16_t productId;
    int timeoutFailures;    // TIMEOUT failures in stage 10-18 (for adding to whitelist)
    int generalFailures;    // ANY failures (for removing from whitelist when already in it)
};

static std::vector<PacingFailureRecord> sPacingFailureRecords;

static PacingFailureRecord * FindOrCreateFailureRecord(uint16_t vendorId, uint16_t productId)
{
    for (auto & r : sPacingFailureRecords)
    {
        if (r.vendorId == vendorId && r.productId == productId)
            return &r;
    }
    sPacingFailureRecords.push_back({vendorId, productId, 0, 0});
    return &sPacingFailureRecords.back();
}

void RecordPacingFailure(uint16_t vendorId, uint16_t productId, uint8_t stageFailed)
{
    // Only consider failures in BLE certificate/data transfer stages (10~18) for ADDING to whitelist
    if (stageFailed < 10 || stageFailed > 18)
    {
        ChipLogDetail(DeviceLayer, "BLE pacing: stage %u outside range 10-18, ignoring for timeout tracking", stageFailed);
        return;
    }

    PacingFailureRecord * record = FindOrCreateFailureRecord(vendorId, productId);
    record->timeoutFailures++;

    bool inWhitelist = IsInPacingWhitelist(vendorId, productId);
    ChipLogProgress(DeviceLayer, "BLE pacing timeout failure: vid=0x%04X pid=0x%04X stage=%u timeoutCount=%d/%d inWhitelist=%d",
                    vendorId, productId, stageFailed, record->timeoutFailures,
                    BLE_PACING_FAILURE_THRESHOLD, inWhitelist ? 1 : 0);

    // Only add to whitelist if not already in it
    if (!inWhitelist && record->timeoutFailures >= BLE_PACING_FAILURE_THRESHOLD)
    {
        ChipLogProgress(DeviceLayer, "BLE pacing: auto-adding vid=0x%04X pid=0x%04X to whitelist (timeout threshold reached)",
                        vendorId, productId);
        AddToPacingWhitelist(vendorId, productId);
        record->timeoutFailures = 0;
        record->generalFailures = 0; // Reset general counter when adding
    }
}

void RecordCommissioningFailure(uint16_t vendorId, uint16_t productId)
{
    // Track ANY commissioning failure for devices IN the whitelist (for removal)
    bool inWhitelist = IsInPacingWhitelist(vendorId, productId);
    if (!inWhitelist)
    {
        // Not in whitelist, nothing to remove
        return;
    }

    PacingFailureRecord * record = FindOrCreateFailureRecord(vendorId, productId);
    record->generalFailures++;

    ChipLogProgress(DeviceLayer, "BLE pacing general failure: vid=0x%04X pid=0x%04X generalCount=%d/%d (in whitelist)",
                    vendorId, productId, record->generalFailures, BLE_PACING_REMOVAL_THRESHOLD);

    if (record->generalFailures >= BLE_PACING_REMOVAL_THRESHOLD)
    {
        ChipLogProgress(DeviceLayer, "BLE pacing: auto-removing vid=0x%04X pid=0x%04X from whitelist (pacing may be causing issues)",
                        vendorId, productId);
        RemoveFromPacingWhitelist(vendorId, productId);
        record->timeoutFailures = 0;
        record->generalFailures = 0;
    }
}

void ClearPacingFailureRecord(uint16_t vendorId, uint16_t productId)
{
    for (auto & r : sPacingFailureRecords)
    {
        if (r.vendorId == vendorId && r.productId == productId)
        {
            if (r.timeoutFailures > 0 || r.generalFailures > 0)
            {
                ChipLogDetail(DeviceLayer, "BLE pacing failure record cleared: vid=0x%04X pid=0x%04X (timeout=%d, general=%d)",
                              vendorId, productId, r.timeoutFailures, r.generalFailures);
            }
            r.timeoutFailures = 0;
            r.generalFailures = 0;
            return;
        }
    }
}

struct ConnectionDataBundle
{
    WbsConnection * mpConn;
    chip::System::PacketBufferHandle buf;
};

static void WbsOTConnectionDestroy(WbsConnection * aConn)
{
    if (aConn)
    {
        // Cancel the subscriptions holding aConn as callback context before releasing it.
        if (aConn->ulMonitorToken != LSMESSAGE_TOKEN_INVALID)
        {
            LsRequester::getInstance()->lsCallCancel(aConn->ulMonitorToken);
        }
        // [LGE_MATTER_COMPAT_PATCH]
        if (aConn->mMtuStatusToken != LSMESSAGE_TOKEN_INVALID)
        {
            LsRequester::getInstance()->lsCallCancel(aConn->mMtuStatusToken);
        }

        if (aConn->mpPeerAddress)
            g_free(aConn->mpPeerAddress);

        chip::Platform::Delete(aConn);
    }
}

// The connection is handed to BLEManagerImpl via CHIPoWbs_ConnectionClosed(), whose event handler
// may still use it (e.g. CloseConnection). Destroy it from a work item queued after that event.
static void WbsOTConnectionDestroyDeferred(WbsConnection * aConn)
{
    CHIP_ERROR err = PlatformMgr().ScheduleWork(
        [](intptr_t arg) { WbsOTConnectionDestroy(reinterpret_cast<WbsConnection *>(arg)); }, reinterpret_cast<intptr_t>(aConn));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to schedule connection release: %" CHIP_ERROR_FORMAT, err.Format());
        WbsOTConnectionDestroy(aConn);
    }
}

void EndpointCleanup(WbsEndpoint * apEndpoint)
{
    if (apEndpoint != nullptr)
    {
        if (apEndpoint->mConnectionMap != nullptr)
        {
            // Release the remaining connections (the map does not own its values).
            GHashTableIter iter;
            gpointer value;
            g_hash_table_iter_init(&iter, apEndpoint->mConnectionMap);
            while (g_hash_table_iter_next(&iter, nullptr, &value))
            {
                // Steal first: the key is owned by the connection.
                g_hash_table_iter_steal(&iter);
                WbsOTConnectionDestroy(static_cast<WbsConnection *>(value));
            }
            g_hash_table_destroy(apEndpoint->mConnectionMap);
            apEndpoint->mConnectionMap = nullptr;
        }
        g_free(apEndpoint);
    }
}

CHIP_ERROR InitConnectionData(bool aIsCentral, WbsEndpoint *& apEndpoint)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool retval              = false;
    WbsEndpoint * endpoint = nullptr;

    // initialize server endpoint
    endpoint = g_new0(WbsEndpoint, 1);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "FAIL: memory allocation in %s", __func__));

    endpoint->mConnectionMap  = g_hash_table_new(g_str_hash, g_str_equal);
    endpoint->mIsCentral = aIsCentral;

    retval = true;

exit:
    if (retval)
    {
        apEndpoint = endpoint;
        ChipLogDetail(DeviceLayer, "InitConnectionData init success");
    }
    else
    {
        EndpointCleanup(endpoint);
    }

    return err;
}

CHIP_ERROR ShutdownWbsLayer(WbsEndpoint * apEndpoint)
{
    VerifyOrReturnError(apEndpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    EndpointCleanup(apEndpoint);
    return CHIP_NO_ERROR;
}

static bool GattGetStatus(std::string address)
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("address", address);
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_GETSTATUS, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return false;
    }
    if(!responsePayload.hasKey("connected") || !responsePayload["connected"].asBool())
    {
        return false;
    }

    return true;
}

static bool GattGetServices(std::string address)
{
    ChipLogProgress(DeviceLayer, "GattGetServices = %s", address.c_str());
    isNotSupportIndication = false;
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("address", address);
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_GETSERVICES, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return false;
    }
    if(responsePayload["services"].arraySize() == 0)
    {
        return false;
    }

    //check :  C2 characteristic properties
    int size = responsePayload["services"].arraySize();
    for(int i = 0; i < size ; i ++){
        ChipLogDetail(DeviceLayer, "receiveMessage [%d] = %s", i,responsePayload["services"][i].stringify().c_str());
        if(responsePayload["services"][i].hasKey("service") == true && (responsePayload["services"][i]["service"].asString().compare(CHIP_BLE_GATT_SERVICE) == 0)) {
            int characteristicsArraySize = responsePayload["services"][i]["characteristics"].arraySize();
                for(int j = 0; j < characteristicsArraySize ; j ++){
                    if(responsePayload["services"][i]["characteristics"][j].hasKey("characteristic") == true
                    && responsePayload["services"][i]["characteristics"][j].hasKey("properties") == true
                    && (responsePayload["services"][i]["characteristics"][j]["characteristic"].asString().compare(CHIP_BLE_GATT_CHAR_READ) == 0)) {
                        bool indicate = responsePayload["services"][i]["characteristics"][j]["properties"]["indicate"].asBool();
                        bool notify =  responsePayload["services"][i]["characteristics"][j]["properties"]["notify"].asBool();
                        if(indicate == false && notify == true) {
                            isNotSupportIndication = true;
                            ChipLogError(DeviceLayer, "[%s] is not support indication!", address.c_str());
                        }
                    }
            }
        }
    }
    return true;
}

/// Update the table of open BLE connections whenever a new device is spotted or its attributes have changed.
static void UpdateConnectionTable(std::string remoteAddr, std::string clientId, WbsEndpoint & aEndpoint)
{
    WbsConnection * connection = static_cast<WbsConnection *>(g_hash_table_lookup(aEndpoint.mConnectionMap, remoteAddr.c_str()));
    bool bConnected = GattGetStatus(remoteAddr);

    if (connection != nullptr && !bConnected)
    {
        ChipLogDetail(DeviceLayer, "Wbs disconnected");
        // 해시맵 키가 connection->mpPeerAddress를 그대로 가리키므로,
        // WbsOTConnectionDestroy로 먼저 해제하면 g_hash_table_remove가 해제된
        // 키를 비교(use-after-free)하게 된다. remove를 destroy보다 먼저 수행한다.
        g_hash_table_remove(aEndpoint.mConnectionMap, remoteAddr.c_str());
        BLEManagerImpl::CHIPoWbs_ConnectionClosed(connection);
        WbsOTConnectionDestroyDeferred(connection);
        return;
    }

    if (connection == nullptr && !bConnected && aEndpoint.mIsCentral)
    {
        return;
    }

    // 이미 엔트리가 있는 주소에 새 connect 요청이 들어오고 링크도 살아있는 경우 →
    // 이전 페어링 시도의 정리되지 못한 잔재로 간주하고 stale 엔트리 제거 후
    // 아래 new-connection 분기로 fall-through 시켜 HandleNewConnection을 다시 발사.
    if (connection != nullptr && bConnected && aEndpoint.mIsCentral)
    {
        ChipLogDetail(DeviceLayer, "Wbs re-init stale connection for %s", remoteAddr.c_str());
        // 해시맵 키가 connection->mpPeerAddress를 그대로 가리키므로,
        // WbsOTConnectionDestroy로 먼저 해제하면 g_hash_table_remove가 해제된
        // 키를 비교(use-after-free)하게 된다. remove를 destroy보다 먼저 수행한다.
        g_hash_table_remove(aEndpoint.mConnectionMap, remoteAddr.c_str());
        BLEManagerImpl::CHIPoWbs_ConnectionClosed(connection);
        WbsOTConnectionDestroyDeferred(connection);
        connection = nullptr;
    }

    if (connection == nullptr && bConnected)// &&
        //(!aEndpoint.mIsCentral || GattGetServices(remoteAddr))) // Delete unnecessary duplicate checks
    {
        // WbsConnection holds std::string/std::atomic members, so it must be constructed (not g_new0).
        connection = chip::Platform::New<WbsConnection>();
        VerifyOrReturn(connection != nullptr, ChipLogError(DeviceLayer, "FAIL: memory allocation in %s", __func__));
        connection->mpPeerAddress = g_strdup(remoteAddr.c_str());
        connection->mpEndpoint    = &aEndpoint;
        connection->clientId      = clientId;
        g_hash_table_insert(aEndpoint.mConnectionMap, connection->mpPeerAddress, connection);

        ChipLogDetail(DeviceLayer, "New BLE connection: conn %p, device %s", connection, connection->mpPeerAddress);

        BLEManagerImpl::HandleNewConnection(connection);
    }
}

static CHIP_ERROR WbsDisconnect(WbsConnection * conn)
{
    bool ret = 0;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    VerifyOrReturnError(conn != nullptr, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "conn is NULL in %s", __func__));
    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, conn->mpPeerAddress);

    // [LGE_MATTER_COMPAT_PATCH]
    if (conn->mMtuStatusToken != LSMESSAGE_TOKEN_INVALID)
    {
        lsRequester->lsCallCancel(conn->mMtuStatusToken);
    }

    lunaParam.put("clientId", conn->clientId);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Drop the entry instead of calling UpdateConnectionTable(): gatt/getStatus may still report
    // "connected" right after gatt/disconnect, which would re-create the connection and report it as new.
    if (g_hash_table_lookup(conn->mpEndpoint->mConnectionMap, conn->mpPeerAddress) == conn)
    {
        g_hash_table_remove(conn->mpEndpoint->mConnectionMap, conn->mpPeerAddress);
        BLEManagerImpl::CHIPoWbs_ConnectionClosed(conn);
        WbsOTConnectionDestroyDeferred(conn);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CloseWbsConnection(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(WbsDisconnect, static_cast<WbsConnection *>(apConn));
}

static CHIP_ERROR SendWriteRequestImpl(ConnectionDataBundle * data)
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    VerifyOrReturnError(data->mpConn != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // [LGE_MATTER_COMPAT_PATCH]
    const int pacingDelayMs = sActivePacingDelayMs.load();
    if (pacingDelayMs > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(pacingDelayMs));
    }

    lunaParam.put("clientId", data->mpConn->clientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
    pbnjson::JValue valueParam = pbnjson::JObject();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    uint8_t * bytes = data->buf->Start();
    for (size_t i = 0; i < data->buf->DataLength(); ++i)
    {
        bytesJArray.append(bytes[i]);
    }
    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITECHRACTERISTIC, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleWriteComplete(data->mpConn);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    // GLibMatterContextInvokeSync() blocks until SendWriteRequestImpl() returns, so the bundle can live on the stack.
    ConnectionDataBundle bundle{ static_cast<WbsConnection *>(apConn), std::move(apBuf) };
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, &bundle);
}

static bool gattMonitorCharateristicsCb(LSHandle * sh, LSMessage * message, void * userData)
{
    WbsConnection * conn = static_cast<WbsConnection *>(userData);
    LS::Message response(message);
    pbnjson::JValue responsePayload;

    responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    VerifyOrExit(responsePayload["returnValue"].asBool() == true,
        ChipLogError(DeviceLayer, "FAIL: WbsSubscribeCharacteristic : %s (%d)", responsePayload["errorText"].asString().c_str(), responsePayload["errorCode"].asNumber<int32_t>()));
    if (responsePayload.hasKey("changed") == true )
    {
        pbnjson::JValueArrayElement bytesDataJObj = responsePayload["changed"]["value"]["bytes"];
        ssize_t bytesDataJSize = bytesDataJObj.arraySize();
        // Max ATT attribute value length; HandleTXCharChanged() copies the data into a packet buffer.
        uint8_t data[512];

        VerifyOrExit(bytesDataJSize >= 0 && static_cast<size_t>(bytesDataJSize) <= sizeof(data),
                     ChipLogError(DeviceLayer, "Invalid TX characteristic value length: %zd", bytesDataJSize));
        for(ssize_t i  = 0; i < bytesDataJSize; ++i)
        {
            int32_t v = 0;
            bytesDataJObj[i].asNumber<int32_t>(v);
            data[i] = chip::CanCastTo<uint8_t>(v) ? static_cast<uint8_t>(v) : 0;
        }

        BLEManagerImpl::HandleTXCharChanged(conn, data, static_cast<size_t>(bytesDataJSize));
    }

exit:
    return true;
}

static CHIP_ERROR SubscribeCharacteristicImpl(WbsConnection * conn)
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue valueParam = pbnjson::JObject();
    pbnjson::JValue charsJArray = pbnjson::JArray();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    pbnjson::JValue responsePayload;

    VerifyOrReturnError(conn != nullptr, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    lunaParam.put("clientId", conn->clientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    charsJArray.append(std::string(CHIP_BLE_GATT_CHAR_READ));

    lunaParam.put("characteristics", charsJArray);
    lunaParam.put("subscribe", true);

    // The token is stored on the connection so that it is cancelled on unsubscribe/destroy.
    ret = lsRequester->lsSubscribe(API_BLUETOOTH_GATT_MONITORCHRACTERISTICS, lunaParam.stringify().c_str(), conn, gattMonitorCharateristicsCb, &conn->ulMonitorToken);
    VerifyOrReturnError(ret == true, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "monitorCharacteristics subscribe failed"));

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    lunaParam.remove("characteristics");
    lunaParam.remove("subscribe");
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));
    lunaParam.put("descriptor", std::string("00002902-0000-1000-8000-00805f9b34fb"));

    if(isNotSupportIndication) {
        //enable notification
        ChipLogError(DeviceLayer,"Workaround : This device is not support indicate.");
        bytesJArray.append(1);//workaround patch
    }
    else { //enable indication
        bytesJArray.append(2); //matter spec
    }
    bytesJArray.append(0);

    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITEDESCRIPTOR, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        lsRequester->lsCallCancel(conn->ulMonitorToken);
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleSubscribeOpComplete(conn, true);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SubscribeCharacteristicImpl, static_cast<WbsConnection *>(apConn));
}

static CHIP_ERROR UnsubscribeCharacteristicImpl(WbsConnection * connection)
{
    CHIP_ERROR result = CHIP_ERROR_INTERNAL;

    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    VerifyOrExit(LsRequester::getInstance()->lsCallCancel(connection->ulMonitorToken) == true,  ChipLogError(DeviceLayer, "lsCallCancel failed") );

    result = CHIP_NO_ERROR;
    BLEManagerImpl::HandleSubscribeOpComplete(connection, false);

exit:
    return result;
}

CHIP_ERROR WbsUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(UnsubscribeCharacteristicImpl, static_cast<WbsConnection *>(apConn));
}

struct ConnectParams
{
    ConnectParams(const char * remoteAddress, WbsEndpoint * endpoint) : mAddress(remoteAddress), mEndpoint(endpoint), mNumRetries(0) {}
    const char * mAddress;
    WbsEndpoint * mEndpoint;
    uint16_t mNumRetries;
};

// [LGE_MATTER_COMPAT_PATCH]
static CHIP_ERROR ConfigureMtuImpl(WbsConnection * conn);

static CHIP_ERROR ConnectDeviceImpl(ConnectParams * apParams)//char * apAddress)
{
    ChipLogProgress(Ble, "ConnectDeviceImpl() start");
    const char * deviceAddress = apParams->mAddress;
    WbsEndpoint * endpoint = apParams->mEndpoint;
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;
    std::string wbsAddress;
    std::string wbsClientId;

    lunaParam.put("address", std::string(deviceAddress));
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_CONNECT, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (responsePayload.hasKey("clientId"))
    {
        wbsClientId = responsePayload["clientId"].asString();
    }
    if (responsePayload.hasKey("address"))
    {
        wbsAddress = responsePayload["address"].asString();
    }
    if (wbsAddress.empty())
    {
        wbsAddress = deviceAddress;
    }

    // gatt/connect succeeded: release the WBS client on any later failure so that a retry does not leak it.
    auto disconnectClient = [&]() {
        pbnjson::JValue disconnectParam = pbnjson::JObject();
        pbnjson::JValue disconnectResponse;
        disconnectParam.put("clientId", wbsClientId);
        if (!lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, disconnectParam.stringify().c_str(), disconnectResponse))
        {
            ChipLogError(Ble, "ConnectDeviceImpl() gatt/disconnect failed for %s", wbsAddress.c_str());
        }
    };

    bool serviceAvailable = false;
    for (int i = 0; i < kMaxConnectRetries ; ++i)
    {
        if(GattGetServices(wbsAddress) == true)
        {
            serviceAvailable = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    if(serviceAvailable == false)
    {
        ChipLogError(Ble, "ConnectDeviceImpl() API_BLUETOOTH_GATT_DISCOVERSERVICES call");

        ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCOVERSERVICES, lunaParam.stringify().c_str(), responsePayload);
        if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool()
           || GattGetServices(wbsAddress) != true)
        {
            disconnectClient();
            return CHIP_ERROR_INTERNAL;
        }
    }

    UpdateConnectionTable(wbsAddress, wbsClientId, *endpoint);

    // [LGE_MATTER_COMPAT_PATCH]
    // Request MTU exchange after connection is established
    {
        WbsConnection * conn = static_cast<WbsConnection *>(
            g_hash_table_lookup(endpoint->mConnectionMap, wbsAddress.c_str()));
        if (conn != nullptr)
        {
            ChipLogProgress(DeviceLayer, "ConnectDeviceImpl: initiating MTU exchange for %s", wbsAddress.c_str());
            TEMPORARY_RETURN_IGNORED ConfigureMtuImpl(conn);
            ChipLogProgress(DeviceLayer, "ConnectDeviceImpl: MTU exchange done, mMtu=%u", conn->mMtu);
        }
        else
        {
            ChipLogError(DeviceLayer, "ConnectDeviceImpl: connection not found in map for MTU exchange");
        }
    }

    return CHIP_NO_ERROR;
}

// [LGE_MATTER_COMPAT_PATCH]
static bool MtuStatusSubscribeCb(LSHandle * sh, LSMessage * message, void * userData)
{
    WbsConnection * conn = static_cast<WbsConnection *>(userData);
    if (conn == nullptr)
        return true;

    LS::Message response(message);
    pbnjson::JValue payload = pbnjson::JDomParser::fromString(response.getPayload());
    if (payload.isValid() && payload.hasKey("mtu"))
    {
        uint16_t mtu = static_cast<uint16_t>(payload["mtu"].asNumber<int32_t>());
        if (mtu > 0)
        {
            conn->mNegotiatedMtu.store(mtu, std::memory_order_release);
        }
    }
    return true;
}

static CHIP_ERROR ConfigureMtuImpl(WbsConnection * conn)
{
    constexpr uint16_t kRequestedMtu = 247;
    bool ret = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    VerifyOrReturnError(conn != nullptr, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    lunaParam.put("clientId", conn->clientId);
    lunaParam.put("mtuSize", static_cast<int32_t>(kRequestedMtu));

    ChipLogProgress(DeviceLayer, "Requesting MTU exchange: clientId=%s peer=%s mtuSize=%u",
                    conn->clientId.c_str(), conn->mpPeerAddress ? conn->mpPeerAddress : "null", kRequestedMtu);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_REQUESTMTU, lunaParam.stringify().c_str(), responsePayload);
    ChipLogDetail(DeviceLayer, "requestMtu response: %s", responsePayload.stringify().c_str());

    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        ChipLogError(DeviceLayer, "requestMtu failed: %s (errorCode=%d)",
                     responsePayload.hasKey("errorText") ? responsePayload["errorText"].asString().c_str() : "unknown",
                     responsePayload.hasKey("errorCode") ? responsePayload["errorCode"].asNumber<int32_t>() : -1);
        // MTU negotiation failure is non-fatal; proceed with default (minimum) MTU
        return CHIP_NO_ERROR;
    }

    // MTU negotiation is asynchronous. The requestMtu response only carries
    // returnValue/clientId (no mtu field); wbs publishes the negotiated MTU to
    // gatt/getStatus subscribers when configure_mtu completes. The BTP handshake
    // fixes its fragment size from GetMTU() only once, so the negotiated MTU must
    // be settled before this function returns.
    //
    // Subscribe to gatt/getStatus once (per-device monitoring, the way wbs intends
    // it), then wait for the callback to publish a non-zero MTU. The callback runs
    // on the LsRequester thread, so this loop only reads the atomic (no repeated
    // IPC). If no MTU arrives within the timeout, leave conn->mMtu untouched (0) so
    // BTP falls back to the minimum MTU. The subscription token is stored on the
    // connection and cancelled on disconnect/destroy.
    {
        constexpr int kMaxWaits       = 10; // total wait = kMaxWaits * kWaitIntervalMs
        constexpr int kWaitIntervalMs = 20; // -> up to 200ms

        // Start the getStatus subscription (reuses/refreshes any existing token).
        pbnjson::JValue statusParam = pbnjson::JObject();
        statusParam.put("address", conn->mpPeerAddress ? conn->mpPeerAddress : "");
        statusParam.put("subscribe", true);
        lsRequester->lsSubscribe(API_BLUETOOTH_GATT_GETSTATUS, statusParam.stringify().c_str(),
                                 conn, MtuStatusSubscribeCb, &conn->mMtuStatusToken);

        int waits = 0; // number of wait iterations actually performed
        uint16_t negotiatedMtu = conn->mNegotiatedMtu.load(std::memory_order_acquire);
        for (; negotiatedMtu == 0 && waits < kMaxWaits; ++waits)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(kWaitIntervalMs));
            negotiatedMtu = conn->mNegotiatedMtu.load(std::memory_order_acquire);
        }

        if (negotiatedMtu > 0)
        {
            conn->mMtu = negotiatedMtu;
            ChipLogProgress(DeviceLayer, "MTU settled via subscribe: peer=%s mtu=%u (waits=%d, ~%dms)",
                            conn->mpPeerAddress ? conn->mpPeerAddress : "null", negotiatedMtu,
                            waits, waits * kWaitIntervalMs);
        }
        else
        {
            // Leave conn->mMtu at its initial value (0) so BTP uses the minimum MTU.
            ChipLogError(DeviceLayer, "MTU subscribe timeout; falling back to minimum MTU (peer=%s, waits=%d)",
                         conn->mpPeerAddress ? conn->mpPeerAddress : "null", waits);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectDevice(std::string address, WbsEndpoint * apEndpoint)
{
    isNotSupportIndication = false;
    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;
    VerifyOrReturnError(apEndpoint != nullptr, CHIP_ERROR_INCORRECT_STATE, BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL));
    // GLibMatterContextInvokeSync() is synchronous, so the parameters can live on the stack.
    ConnectParams params(address.c_str(), apEndpoint);

    while (params.mNumRetries ++ < kMaxConnectRetries)
    {
        if (PlatformMgrImpl().GLibMatterContextInvokeSync(ConnectDeviceImpl, &params) == CHIP_NO_ERROR)
        {
            err = CHIP_NO_ERROR;
            ChipLogProgress(Ble, "ConnectDeviceImpl() Success");
            break;
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            ChipLogError(Ble, "ConnectDeviceImpl() failed (attempt %u/%u)", params.mNumRetries, kMaxConnectRetries);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    if (err != CHIP_NO_ERROR)
        BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);

    return err;
}

void CancelConnect(WbsEndpoint * apEndpoint)
{
    ChipLogDetail(Ble, "%s : NOT Implemented", __func__);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
