#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <lib/support/SafeInt.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <luna-service2++/handle.hpp>
#include "lsrequester.h"
#include "Helper.h"

#define API_BLUETOOTH_GATT_GETSTATUS "luna://com.webos.service.bluetooth2/gatt/getStatus"
#define API_BLUETOOTH_GATT_CONNECT "luna://com.webos.service.bluetooth2/gatt/connect"
#define API_BLUETOOTH_GATT_DISCONNECT "luna://com.webos.service.bluetooth2/gatt/disconnect"
#define API_BLUETOOTH_GATT_DISCOVERSERVICES "luna://com.webos.service.bluetooth2/gatt/discoverServices"
#define API_BLUETOOTH_GATT_GETSERVICES "luna://com.webos.service.bluetooth2/gatt/getServices"

#define API_BLUETOOTH_GATT_MONITORCHRACTERISTICS "luna://com.webos.service.bluetooth2/gatt/monitorCharacteristics"
#define API_BLUETOOTH_GATT_WRITEDESCRIPTOR "luna://com.webos.service.bluetooth2/gatt/writeDescriptorValue"
#define API_BLUETOOTH_GATT_WRITECHRACTERISTIC "luna://com.webos.service.bluetooth2/gatt/writeCharacteristicValue"

#define CHIP_BLE_GATT_SERVICE "0000fff6-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_GATT_CHAR_WRITE "18ee2ef5-263d-4559-959f-4f9c429f9d11"
#define CHIP_BLE_GATT_CHAR_READ "18ee2ef5-263d-4559-959f-4f9c429f9d12"

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kMaxConnectRetries = 4;

struct ConnectionDataBundle
{
    WbsConnection * mpConn;
    chip::System::PacketBufferHandle buf;
};

void EndpointCleanup(WbsEndpoint * apEndpoint)
{
    if (apEndpoint != nullptr)
    {
        if (apEndpoint->mConnectionMap != nullptr)
        {
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

static void WbsOTConnectionDestroy(WbsConnection * aConn)
{
    if (aConn)
    {
        if (aConn->mpPeerAddress)
            g_free(aConn->mpPeerAddress);

        g_free(aConn);
    }
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
        BLEManagerImpl::CHIPoWbs_ConnectionClosed(connection);
        // TODO: the connection object should be released after BLEManagerImpl finishes cleaning up its resources
        // after the disconnection. Releasing it here doesn't cause any issues, but it's error-prone.
        WbsOTConnectionDestroy(connection);
        g_hash_table_remove(aEndpoint.mConnectionMap, remoteAddr.c_str());
        return;
    }

    if (connection == nullptr && !bConnected && aEndpoint.mIsCentral)
    {
        return;
    }

    if (connection == nullptr && bConnected &&
        (!aEndpoint.mIsCentral || GattGetServices(remoteAddr)))
    {
        connection                = g_new0(WbsConnection, 1);
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

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "conn is NULL in %s", __func__));
    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, conn->mpPeerAddress);

    lunaParam.put("clientId", conn->clientId);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return CHIP_ERROR_INTERNAL;
    }

    UpdateConnectionTable(conn->mpPeerAddress, conn->clientId, *(conn->mpEndpoint));

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR CloseWbsConnection(BLE_CONNECTION_OBJECT apConn)
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(WbsDisconnect, static_cast<WbsConnection *>(apConn));
}

static ConnectionDataBundle * MakeConnectionDataBundle(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    ConnectionDataBundle * bundle = g_new(ConnectionDataBundle, 1);
    bundle->mpConn                = static_cast<WbsConnection *>(apConn);
    bundle->buf = std::move(apBuf);

    return bundle;
}

static CHIP_ERROR SendWriteRequestImpl(ConnectionDataBundle * data)
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("clientId", data->mpConn->clientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
    pbnjson::JValue valueParam = pbnjson::JObject();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    uint8_t * bytes = data->buf->Start();
    for (uint16_t i = 0; i < data->buf->DataLength(); ++i)
    {
        bytesJArray.append(bytes[i]);
    }
    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITECHRACTERISTIC, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        g_free(data);
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleWriteComplete(data->mpConn);

    g_free(data);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, MakeConnectionDataBundle(apConn, std::move(apBuf)));
}

static bool gattMonitorCharateristicsCb(LSHandle * sh, LSMessage * message, void * userData)
{
    WbsConnection * conn = static_cast<WbsConnection *>(userData);
    LS::Message response(message);
    pbnjson::JValue responsePayload;
    std::string responseStr(response.getPayload());
    System::PacketBufferHandle buf;

    responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    VerifyOrExit(responsePayload["returnValue"].asBool() == true,
        ChipLogError(DeviceLayer, "FAIL: WbsSubscribeCharacteristic : %s (%d)", responsePayload["errorText"].asString().c_str(), responsePayload["errorCode"].asNumber<int32_t>()));
    if (responsePayload.hasKey("changed") == true )
    {
        pbnjson::JValueArrayElement bytesDataJObj = responsePayload["changed"]["value"]["bytes"];
        ssize_t bytesDataJSize = bytesDataJObj.arraySize();
        int32_t v;

        buf = System::PacketBufferHandle::New(bytesDataJSize, 0);
        for(ssize_t i  = 0; i < bytesDataJSize; ++i)
        {
            bytesDataJObj[i].asNumber<int32_t>(v);
            if (chip::CanCastTo<uint8_t>(v))
                *(buf->Start() + i) = static_cast<uint8_t>(v);
        }
        buf->SetDataLength(bytesDataJSize);

        BLEManagerImpl::HandleTXCharChanged(conn, buf->Start(), buf->DataLength());
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
    LSMessageToken ulToken = LSMESSAGE_TOKEN_INVALID;

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    lunaParam.put("clientId", conn->clientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    charsJArray.append(std::string(CHIP_BLE_GATT_CHAR_READ));

    lunaParam.put("characteristics", charsJArray);
    lunaParam.put("subscribe", true);

    ret = lsRequester->lsSubscribe(API_BLUETOOTH_GATT_MONITORCHRACTERISTICS, lunaParam.stringify().c_str(), conn, gattMonitorCharateristicsCb, &ulToken);

    conn->ulMonitorToken = ulToken;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    lunaParam.remove("characteristics");
    lunaParam.remove("subscribe");
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));
    lunaParam.put("descriptor", std::string("00002902-0000-1000-8000-00805f9b34fb"));

    bytesJArray.append(2);
    bytesJArray.append(0);

    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITEDESCRIPTOR, lunaParam.stringify().c_str(), responsePayload);
    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        lsRequester->lsCallCancel(ulToken);
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleSubscribeOpComplete(conn, true);

exit:
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
    ConnectParams(char * remoteAddress, WbsEndpoint * endpoint) : mAddress(remoteAddress), mEndpoint(endpoint), mNumRetries(0) {}
    char * mAddress;
    WbsEndpoint * mEndpoint;
    uint16_t mNumRetries;
};

static CHIP_ERROR ConnectDeviceImpl(ConnectParams * apParams)//char * apAddress)
{
    char * deviceAddress    = apParams->mAddress;
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
        ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCOVERSERVICES, lunaParam.stringify().c_str(), responsePayload);
        if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if(GattGetServices(wbsAddress) != true)
            return CHIP_ERROR_INTERNAL;
    }

    UpdateConnectionTable(wbsAddress, wbsClientId, *endpoint);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectDevice(std::string address, WbsEndpoint * apEndpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char * mRemoteAddress = chip::Platform::MemoryAllocString(address.c_str(), address.length());
    auto params = chip::Platform::New<ConnectParams>(mRemoteAddress, apEndpoint);

    while (params->mNumRetries ++ < kMaxConnectRetries)
    {
        if (PlatformMgrImpl().GLibMatterContextInvokeSync(ConnectDeviceImpl, params) == CHIP_NO_ERROR)
        {
            err = CHIP_NO_ERROR;
            ChipLogError(Ble, "ConnectDeviceImpl() Success");
            break;
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            ChipLogError(Ble, "Failed to schedule ConnectDeviceImpl() on gmain-matter thread");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    chip::Platform::Delete(mRemoteAddress);
    chip::Platform::Delete(params);
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
