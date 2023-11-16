/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include "BluezAdvertisement.h"

#include <memory>
#include <unistd.h>

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConfigurationManager.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <platform/PlatformManager.h>

#include "BluezEndpoint.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

BluezLEAdvertisement1 * BluezAdvertisement::CreateLEAdvertisement()
{
    BluezLEAdvertisement1 * adv;
    BluezObjectSkeleton * object;
    GVariant * serviceData;
    GVariant * serviceUUID;
    GVariantBuilder serviceDataBuilder;
    GVariantBuilder serviceUUIDsBuilder;
    GAutoPtr<char> debugStr;
    const char * localNamePtr;
    char localName[32];

    ChipLogDetail(DeviceLayer, "Create BLE adv object at %s", mpAdvPath);
    object = bluez_object_skeleton_new(mpAdvPath);

    adv = bluez_leadvertisement1_skeleton_new();

    g_variant_builder_init(&serviceDataBuilder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_init(&serviceUUIDsBuilder, G_VARIANT_TYPE("as"));

    g_variant_builder_add(&serviceDataBuilder, "{sv}", mpAdvUUID,
                          g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &mDeviceIdInfo, sizeof(mDeviceIdInfo), sizeof(uint8_t)));
    g_variant_builder_add(&serviceUUIDsBuilder, "s", mpAdvUUID);

    localNamePtr = mpAdapterName;
    if (localNamePtr == nullptr)
    {
        g_snprintf(localName, sizeof(localName), "%s%04x", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, getpid() & 0xffff);
        localNamePtr = localName;
    }

    serviceData = g_variant_builder_end(&serviceDataBuilder);
    serviceUUID = g_variant_builder_end(&serviceUUIDsBuilder);

    debugStr = GAutoPtr<char>(g_variant_print(serviceData, TRUE));
    ChipLogDetail(DeviceLayer, "SET service data to %s", StringOrNullMarker(debugStr.get()));

    bluez_leadvertisement1_set_type_(adv, (mAdvType & BLUEZ_ADV_TYPE_CONNECTABLE) ? "peripheral" : "broadcast");
    // empty manufacturer data
    // empty solicit UUIDs
    bluez_leadvertisement1_set_service_data(adv, serviceData);
    // empty data

    // Setting "Discoverable" to False on the adapter and to True on the advertisement convinces
    // Bluez to set "BR/EDR Not Supported" flag. Bluez doesn't provide API to do that explicitly
    // and the flag is necessary to force using LE transport.
    bluez_leadvertisement1_set_discoverable(adv, (mAdvType & BLUEZ_ADV_TYPE_SCANNABLE) ? TRUE : FALSE);
    if (mAdvType & BLUEZ_ADV_TYPE_SCANNABLE)
        bluez_leadvertisement1_set_discoverable_timeout(adv, UINT16_MAX);

    // advertising name corresponding to the PID and object path, for debug purposes
    bluez_leadvertisement1_set_local_name(adv, localNamePtr);
    bluez_leadvertisement1_set_service_uuids(adv, serviceUUID);

    // 0xffff means no appearance
    bluez_leadvertisement1_set_appearance(adv, 0xffff);

    bluez_leadvertisement1_set_duration(adv, mAdvDurationMs);
    // empty duration, we don't have a clear notion what it would mean to timeslice between toble and anyone else
    bluez_leadvertisement1_set_timeout(adv, 0);
    // empty secondary channel for now

    bluez_object_skeleton_set_leadvertisement1(object, adv);
    g_signal_connect(adv, "handle-release",
                     G_CALLBACK(+[](BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInv, BluezAdvertisement * self) {
                         return self->BluezLEAdvertisement1Release(aAdv, aInv);
                     }),
                     this);

    g_dbus_object_manager_server_export(mpRoot, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    return adv;
}

gboolean BluezAdvertisement::BluezLEAdvertisement1Release(BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInvocation)
{
    ChipLogDetail(DeviceLayer, "Release BLE adv object in %s", __func__);
    g_dbus_object_manager_server_unexport(mpRoot, mpAdvPath);
    g_object_unref(mpAdv);
    mpAdv          = nullptr;
    mIsAdvertising = false;
    return TRUE;
}

CHIP_ERROR BluezAdvertisement::InitImpl()
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    mpAdv = CreateLEAdvertisement();
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezAdvertisement::Init(const BluezEndpoint & aEndpoint, ChipAdvType aAdvType, const char * aAdvUUID,
                                    uint32_t aAdvDurationMs)
{
    GAutoPtr<char> rootPath;
    CHIP_ERROR err;

    VerifyOrExit(mpAdv == nullptr, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: BLE advertisement already initialized in %s", __func__));

    mpRoot        = reinterpret_cast<GDBusObjectManagerServer *>(g_object_ref(aEndpoint.GetGattApplicationObjectManager()));
    mpAdapter     = reinterpret_cast<BluezAdapter1 *>(g_object_ref(aEndpoint.GetAdapter()));
    mpAdapterName = g_strdup(aEndpoint.GetAdapterName());

    g_object_get(G_OBJECT(mpRoot), "object-path", &MakeUniquePointerReceiver(rootPath).Get(), nullptr);
    mpAdvPath      = g_strdup_printf("%s/advertising", rootPath.get());
    mAdvType       = aAdvType;
    mpAdvUUID      = g_strdup(aAdvUUID);
    mAdvDurationMs = aAdvDurationMs;

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    ReturnErrorOnFailure(err);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    mDeviceIdInfo.SetAdditionalDataFlag(true);
#endif

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezAdvertisement * self) { return self->InitImpl(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Ble, "Failed to schedule BLE advertisement Init() on CHIPoBluez thread"));

    mIsInitialized = true;

exit:
    return err;
}

void BluezAdvertisement::Shutdown()
{
    VerifyOrReturn(mIsInitialized);

    // Make sure that the advertisement is stopped before we start cleaning up.
    if (mIsAdvertising)
    {
        CHIP_ERROR err = Stop();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to stop BLE advertisement before shutdown: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    // Release resources on the glib thread to synchronize with potential signal handlers
    // attached to the advertising object that may run on the glib thread.
    PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezAdvertisement * self) {
            if (self->mpRoot != nullptr)
            {
                g_object_unref(self->mpRoot);
                self->mpRoot = nullptr;
            }
            if (self->mpAdapter != nullptr)
            {
                g_object_unref(self->mpAdapter);
                self->mpAdapter = nullptr;
            }
            if (self->mpAdv != nullptr)
            {
                g_object_unref(self->mpAdv);
                self->mpAdv = nullptr;
            }
            return CHIP_NO_ERROR;
        },
        this);

    g_free(mpAdvPath);
    mpAdvPath = nullptr;
    g_free(mpAdapterName);
    mpAdapterName = nullptr;
    g_free(mpAdvUUID);
    mpAdvUUID = nullptr;

    mIsInitialized = false;
}

void BluezAdvertisement::StartDone(GObject * aObject, GAsyncResult * aResult)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    GAutoPtr<GError> error;
    gboolean success = FALSE;

    success =
        bluez_leadvertising_manager1_call_register_advertisement_finish(advMgr, aResult, &MakeUniquePointerReceiver(error).Get());
    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(mpRoot, mpAdvPath);
    }
    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: RegisterAdvertisement : %s", error->message));

    mIsAdvertising = true;

    ChipLogDetail(DeviceLayer, "RegisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(success == TRUE, nullptr);
}

CHIP_ERROR BluezAdvertisement::StartImpl()
{
    GDBusObject * adapter;
    BluezLEAdvertisingManager1 * advMgr = nullptr;
    GVariantBuilder optionsBuilder;
    GVariant * options;

    VerifyOrExit(!mIsAdvertising, ChipLogError(DeviceLayer, "FAIL: Advertising has already been enabled in %s", __func__));
    VerifyOrExit(mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(mpAdapter));
    VerifyOrExit(adapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));
    options = g_variant_builder_end(&optionsBuilder);

    bluez_leadvertising_manager1_call_register_advertisement(
        advMgr, mpAdvPath, options, nullptr,
        [](GObject * aObject, GAsyncResult * aResult, void * aData) {
            reinterpret_cast<BluezAdvertisement *>(aData)->StartDone(aObject, aResult);
        },
        this);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezAdvertisement::Start()
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezAdvertisement * self) { return self->StartImpl(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Ble, "Failed to schedule BLE advertisement Start() on CHIPoBluez thread"));
    return err;
}

void BluezAdvertisement::StopDone(GObject * aObject, GAsyncResult * aResult)
{
    BluezLEAdvertisingManager1 * advMgr = BLUEZ_LEADVERTISING_MANAGER1(aObject);
    GAutoPtr<GError> error;
    gboolean success = FALSE;

    success =
        bluez_leadvertising_manager1_call_unregister_advertisement_finish(advMgr, aResult, &MakeUniquePointerReceiver(error).Get());

    if (success == FALSE)
    {
        g_dbus_object_manager_server_unexport(mpRoot, mpAdvPath);
    }
    else
    {
        mIsAdvertising = false;
    }

    VerifyOrExit(success == TRUE, ChipLogError(DeviceLayer, "FAIL: UnregisterAdvertisement : %s", error->message));

    ChipLogDetail(DeviceLayer, "UnregisterAdvertisement complete");

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(success == TRUE, nullptr);
}

CHIP_ERROR BluezAdvertisement::StopImpl()
{
    GDBusObject * adapter;
    BluezLEAdvertisingManager1 * advMgr = nullptr;

    VerifyOrExit(mIsAdvertising, ChipLogError(DeviceLayer, "FAIL: Advertising has already been disabled in %s", __func__));
    VerifyOrExit(mpAdapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL mpAdapter in %s", __func__));

    adapter = g_dbus_interface_get_object(G_DBUS_INTERFACE(mpAdapter));
    VerifyOrExit(adapter != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL adapter in %s", __func__));

    advMgr = bluez_object_get_leadvertising_manager1(BLUEZ_OBJECT(adapter));
    VerifyOrExit(advMgr != nullptr, ChipLogError(DeviceLayer, "FAIL: NULL advMgr in %s", __func__));

    bluez_leadvertising_manager1_call_unregister_advertisement(
        advMgr, mpAdvPath, nullptr,
        [](GObject * aObject, GAsyncResult * aResult, void * aData) {
            reinterpret_cast<BluezAdvertisement *>(aData)->StopDone(aObject, aResult);
        },
        this);

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BluezAdvertisement::Stop()
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](BluezAdvertisement * self) { return self->StopImpl(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Ble, "Failed to schedule BLE advertisement Stop() on CHIPoBluez thread"));
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
