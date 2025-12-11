#include <ble/Ble.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/core/SafeInt.h>

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include "WbsConnection.h"
#include <luna-service2++/handle.hpp>

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

WbsConnection::WbsConnection() {}

CHIP_ERROR WbsConnection::InitConnectionData(bool aIsCentral, WbsEndpoint *& apEndpoint)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool retval            = false;
    WbsEndpoint * endpoint = nullptr;

    // initialize server endpoint
    endpoint = g_new0(WbsEndpoint, 1);
    VerifyOrExit(endpoint != nullptr, ChipLogError(DeviceLayer, "FAIL: memory allocation in %s", __func__));

    endpoint->mConnectionMap = g_hash_table_new(g_str_hash, g_str_equal);
    endpoint->mIsCentral     = aIsCentral;

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
    ChipLogDetail(DeviceLayer, "Wbs InitConnectionData err\n");
    return err;
}

CHIP_ERROR WbsConnection::SendIndication(chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    // ConnectionDataBundle bundle(*this, apBuf);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendIndicationImpl, MakeConnectionDataBundle(this, std::move(apBuf)));
    // return PlatformMgrImpl().GLibMatterContextInvokeSync(SendIndicationImpl, &bundle);
}

CHIP_ERROR WbsConnection::SendIndicationImpl(ConnectionDataBundle * data)
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("clientId", data->mConn->mClientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
    pbnjson::JValue valueParam  = pbnjson::JObject();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    uint8_t * bytes             = data->buf->Start();
    for (uint16_t i = 0; i < data->buf->DataLength(); ++i)
    {
        bytesJArray.append(bytes[i]);
    }
    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITECHRACTERISTIC, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        g_free(data);
        ChipLogError(DeviceLayer, "SendIndicationImpl API_BLUETOOTH_GATT_WRITECHRACTERISTIC Failed");
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleWriteComplete(data->mConn);
    ChipLogDetail(DeviceLayer, "SendIndicationImpl success");
    g_free(data);
    return CHIP_NO_ERROR;
}
CHIP_ERROR WbsConnection::SendWriteRequest(chip::System::PacketBufferHandle apBuf)
{
    VerifyOrReturnError(!apBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(DeviceLayer, "apBuf is NULL in %s", __func__));
    // ConnectionDataBundle bundle(*this, apBuf);
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, MakeConnectionDataBundle(this, std::move(apBuf)));
    // return PlatformMgrImpl().GLibMatterContextInvokeSync(SendWriteRequestImpl, &bundle);
}

CHIP_ERROR WbsConnection::SendWriteRequestImpl(ConnectionDataBundle * data)
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("clientId", data->mConn->mClientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
    pbnjson::JValue valueParam  = pbnjson::JObject();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    uint8_t * bytes             = data->buf->Start();
    for (uint16_t i = 0; i < data->buf->DataLength(); ++i)
    {
        bytesJArray.append(bytes[i]);
    }
    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITECHRACTERISTIC, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        g_free(data);
        ChipLogError(DeviceLayer, "SendWriteRequestImpl API_BLUETOOTH_GATT_WRITECHRACTERISTIC Failed");
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::HandleWriteComplete(data->mConn);
    // ChipLogDetail(DeviceLayer, "SendWriteRequestImpl success");
    g_free(data);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsConnection::ShutdownWbsLayer(WbsEndpoint * apEndpoint)
{
    VerifyOrReturnError(apEndpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    EndpointCleanup(apEndpoint);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsConnection::CloseConnection()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(CloseConnectionImpl, this);
}

CHIP_ERROR WbsConnection::CloseConnectionImpl(WbsConnection * conn)
{
    bool ret                  = 0;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "conn is NULL in %s", __func__));
    ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, conn->mPeerAddress);

    lunaParam.put("clientId", conn->mClientId);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return CHIP_ERROR_INTERNAL;
    }

    UpdateConnectionTable(conn->mPeerAddress, conn->mClientId, *(conn->mEndpoint));

exit:
    return CHIP_NO_ERROR;
}
/////////////////////////// Static //////////////////////////////////

bool WbsConnection::GattGetStatus(std::string address)
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("address", address);
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_GETSTATUS, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return false;
    }
    if (!responsePayload.hasKey("connected") || !responsePayload["connected"].asBool())
    {
        return false;
    }

    return true;
}

bool WbsConnection::GattGetServices(std::string address)
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("address", address);
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_GETSERVICES, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        return false;
    }
    if (responsePayload["services"].arraySize() == 0)
    {
        return false;
    }

    return true;
}

ConnectionDataBundle * WbsConnection::MakeConnectionDataBundle(WbsConnection * aConn, chip::System::PacketBufferHandle aBuf)
{
    ConnectionDataBundle * bundle = g_new(ConnectionDataBundle, 1);
    bundle->mConn                 = aConn;
    bundle->buf                   = std::move(aBuf);
    return bundle;
}

CHIP_ERROR WbsConnection::SubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(SubscribeCharacteristicImpl, this);
}

CHIP_ERROR WbsConnection::SubscribeCharacteristicImpl(BLE_CONNECTION_OBJECT conn)
{
    bool ret                    = false;
    LsRequester * lsRequester   = LsRequester::getInstance();
    pbnjson::JValue lunaParam   = pbnjson::JObject();
    pbnjson::JValue valueParam  = pbnjson::JObject();
    pbnjson::JValue charsJArray = pbnjson::JArray();
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    pbnjson::JValue responsePayload;
    LSMessageToken ulToken = LSMESSAGE_TOKEN_INVALID;

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    lunaParam.put("clientId", conn->mClientId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    charsJArray.append(std::string(CHIP_BLE_GATT_CHAR_READ));

    lunaParam.put("characteristics", charsJArray);
    lunaParam.put("subscribe", true);

    ret = lsRequester->lsSubscribe(API_BLUETOOTH_GATT_MONITORCHRACTERISTICS, lunaParam.stringify().c_str(), conn,
                                   gattMonitorCharateristicsCb, &ulToken);

    conn->mMonitorToken = ulToken;

    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // To Prevent Crash between WBS <-> BLE

    lunaParam.remove("characteristics");
    lunaParam.remove("subscribe");
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));
    lunaParam.put("descriptor", std::string("00002902-0000-1000-8000-00805f9b34fb"));

    bytesJArray.append(2);
    bytesJArray.append(0);

    valueParam.put("bytes", bytesJArray);
    lunaParam.put("value", valueParam);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_WRITEDESCRIPTOR, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        lsRequester->lsCallCancel(ulToken);
        return CHIP_ERROR_INTERNAL;
    }
    // ChipLogDetail(DeviceLayer, "SubscribeCharacteristicImpl success");
    BLEManagerImpl::HandleSubscribeOpComplete(conn, true);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsConnection::UnsubscribeCharacteristic()
{
    return PlatformMgrImpl().GLibMatterContextInvokeSync(UnsubscribeCharacteristicImpl, this);
}

CHIP_ERROR WbsConnection::UnsubscribeCharacteristicImpl(BLE_CONNECTION_OBJECT connection)
{
    CHIP_ERROR result = CHIP_ERROR_INTERNAL;

    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "WbsConnection is NULL in %s", __func__));

    VerifyOrExit(LsRequester::getInstance()->lsCallCancel(connection->mMonitorToken) == true,
                 ChipLogError(DeviceLayer, "lsCallCancel failed"));

    result = CHIP_NO_ERROR;
    BLEManagerImpl::HandleSubscribeOpComplete(connection, false);

exit:
    return result;
}

bool WbsConnection::gattMonitorCharateristicsCb(LSHandle * sh, LSMessage * message, void * userData)
{
    WbsConnection * conn = static_cast<WbsConnection *>(userData);
    LS::Message response(message);
    pbnjson::JValue responsePayload;
    std::string responseStr(response.getPayload());
    System::PacketBufferHandle buf;

    responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    VerifyOrExit(responsePayload["returnValue"].asBool() == true,
                 ChipLogError(DeviceLayer, "FAIL: WbsSubscribeCharacteristic : %s (%d)",
                              responsePayload["errorText"].asString().c_str(), responsePayload["errorCode"].asNumber<int32_t>()));
    if (responsePayload.hasKey("changed") == true)
    {
        pbnjson::JValueArrayElement bytesDataJObj = responsePayload["changed"]["value"]["bytes"];
        ssize_t bytesDataJSize                    = bytesDataJObj.arraySize();
        int32_t v;

        buf = System::PacketBufferHandle::New(bytesDataJSize, 0);
        for (ssize_t i = 0; i < bytesDataJSize; ++i)
        {
            bytesDataJObj[i].asNumber<int32_t>(v);
            if (chip::CanCastTo<uint8_t>(v))
                *(buf->Start() + i) = static_cast<uint8_t>(v);
        }
        buf->SetDataLength(bytesDataJSize);
        // ChipLogDetail(DeviceLayer, "gattMonitorCharateristicsCb success");
        BLEManagerImpl::HandleTXCharChanged(conn, buf->Start(), buf->DataLength());
    }

exit:
    return true;
}

void WbsConnection::WbsOTConnectionDestroy(WbsConnection * aConn)
{
    if (aConn)
    {
        if (aConn->mPeerAddress)
            g_free(aConn->mPeerAddress);

        g_free(aConn);
    }
}

void WbsConnection::EndpointCleanup(WbsEndpoint * apEndpoint)
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

CHIP_ERROR WbsConnection::ConnectDeviceImpl(ConnectParams * apParams)
{
    char * deviceAddress      = apParams->mAddress;
    WbsEndpoint * endpoint    = apParams->mEndpoint;
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;
    std::string wbsAddress;
    std::string wbsClientId;

    lunaParam.put("address", std::string(deviceAddress));
    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_CONNECT, lunaParam.stringify().c_str(), responsePayload);
    if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        ChipLogError(Ble, "ConnectDeviceImpl() Failed API_BLUETOOTH_GATT_CONNECT response %s", responsePayload.asString().c_str());
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
    for (int i = 0; i < kMaxConnectRetries; ++i)
    {
        if (GattGetServices(wbsAddress) == true)
        {
            serviceAvailable = true;
            break;
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (serviceAvailable == false)
    {
        ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCOVERSERVICES, lunaParam.stringify().c_str(), responsePayload);
        if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if (GattGetServices(wbsAddress) != true)
            return CHIP_ERROR_INTERNAL;
    }

    UpdateConnectionTable(wbsAddress, wbsClientId, *endpoint);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsConnection::ConnectDevice(std::string address, WbsEndpoint * aEndpoint)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    char * mRemoteAddress = chip::Platform::MemoryAllocString(address.c_str(), address.length());
    auto params           = chip::Platform::New<ConnectParams>(mRemoteAddress, aEndpoint);
    while (params->mNumRetries++ < kMaxConnectRetries)
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
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    chip::Platform::Delete(mRemoteAddress);
    chip::Platform::Delete(params);
    if (err != CHIP_NO_ERROR)
        BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);

    return err;
}

void WbsConnection::CancelConnect()
{
    if (!mClientId.empty())
    {
        bool ret                  = false;
        LsRequester * lsRequester = LsRequester::getInstance();
        pbnjson::JValue lunaParam = pbnjson::JObject();
        pbnjson::JValue responsePayload;
        ChipLogDetail(DeviceLayer, "%s peer=%s", __func__, mPeerAddress);
        lunaParam.put("clientId", mClientId);

        ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, lunaParam.stringify().c_str(), responsePayload);
        if (ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
        {
            ChipLogError(Ble, "%s API_BLUETOOTH_GATT_DISCONNECT Failed", __func__);
        }
        else
            ChipLogDetail(Ble, "%s : Successed", __func__);
    }
    else
    {
        ChipLogError(Ble, "%s Client ID is empty()", __func__);
    }
    return;
}

void WbsConnection::UpdateConnectionTable(std::string remoteAddr, std::string clientId, WbsEndpoint & aEndpoint)
{
    WbsConnection * connection = static_cast<WbsConnection *>(g_hash_table_lookup(aEndpoint.mConnectionMap, remoteAddr.c_str()));
    bool bConnected            = GattGetStatus(remoteAddr);

    if (connection != nullptr && !bConnected)
    {
        ChipLogDetail(DeviceLayer, "Wbs disconnected");
        BLEManagerImpl::HandleConnectionClosed(connection);
        WbsOTConnectionDestroy(connection);
        g_hash_table_remove(aEndpoint.mConnectionMap, remoteAddr.c_str());
        return;
    }

    if (connection == nullptr && !bConnected && aEndpoint.mIsCentral)
    {
        return;
    }

    if (connection == nullptr && bConnected && (!aEndpoint.mIsCentral || GattGetServices(remoteAddr)))
    {
        connection               = g_new0(WbsConnection, 1);
        connection->mPeerAddress = g_strdup(remoteAddr.c_str()); // mpPeerAddress -> mPeerAddress
        connection->mEndpoint    = &aEndpoint;                   // mpEndpoint -> mEndpoint
        connection->mClientId    = clientId;
        g_hash_table_insert(aEndpoint.mConnectionMap, connection->mPeerAddress, connection); // mpPeerAddress -> mPeerAddress

        ChipLogDetail(DeviceLayer, "New BLE connection: conn %p, device %s", connection,
                      connection->mPeerAddress); // mpPeerAddress -> mPeerAddress

        BLEManagerImpl::HandleNewConnection(connection);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
