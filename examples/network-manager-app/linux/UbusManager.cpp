/*
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include "UbusManager.h"

#include "UboxUtils.h"
#include "UloopHandler.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>

#include <assert.h>
#include <type_traits>

#ifndef CHIP_UBUS_DEBUGGING
#define CHIP_UBUS_DEBUGGING 0
#endif

namespace chip {
namespace ubus {

inline constexpr auto kReconnectTimeout = System::Clock::Seconds16(2);

namespace {
bool CheckAndLog(int status, const char * func)
{
    if (status != UBUS_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "Ubus error: %s() %s", func, ubus_strerror(status));
        return false;
    }
#if CHIP_UBUS_DEBUGGING
    ChipLogDetail(DeviceLayer, "%s() OK", func);
#endif
    return true;
}

} // namespace

CHIP_ERROR UbusManager::Init()
{
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    UloopHandler::Register();
    if (!Connect())
    {
        UloopHandler::Unregister();
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogDetail(DeviceLayer, "Connected to ubus");
    mInitialized = true;
    return CHIP_NO_ERROR;
}

void UbusManager::Shutdown()
{
    VerifyOrReturn(mInitialized);
    mWatches.Clear();
    if (Connected())
    {
        ubus_shutdown(&Context());
    }
    else
    {
        uloop_timeout_cancel(&ReconnectTimer());
    }
    UloopHandler::Unregister();
    mInitialized = false;
}

void UbusManager::HandleConnectionLost()
{
    ChipLogProgress(DeviceLayer, "Ubus connection lost, reconnection will be attempted periodically");
    ubus_shutdown(&Context());

    ResetEventHandler();
    for (auto & watch : mWatches)
    {
        if (watch.mID != 0)
        {
            Lost(watch);
        }
        watch.ResetSubscriber();
    }

    AttemptReconnect(); // side effect: Connected() = false
}

void UbusManager::AttemptReconnect()
{
    // Use ubus_connect_ctx() instead of ubus_reconnect(). The latter provides support for
    // automatic re-registration of objects and re-establishment of subscriptions (those with
    // a non-null ubus_subscriber.new_obj_cb), however these mechanisms have some important
    // limitations:
    //  - The resolved object id of the target object(s) for an ubus_subcriber is not exposed,
    //    but since we want to be able to perform method calls on target objects, the automatic
    //    resubscription support is not sufficient to implement UbusWatch.
    //  - There is no API for removing objects (including subscribers / event listeners) from the
    //    ubus_context while not connected to ubusd, making it impossible to safely free their
    //    ubus_object in that state.
    // Using ubus_connect_ctx() re-initializes the context, and any relevant objects (event
    // listeners, subscriptions, ...) are then re-established explicitly.
    if (!Connect())
    {
        ReconnectTimer().cb = [](uloop_timeout * timeout) {
            static_cast<UbusManager *>(timeout)->AttemptReconnect(); // downcast from base class
        };
        uloop_timeout_set(&ReconnectTimer(), std::chrono::duration_cast<System::Clock::Milliseconds32>(kReconnectTimeout).count());
        return;
    }

    ChipLogProgress(DeviceLayer, "Ubus connection restored");
    ReconnectTimer().cb = nullptr; // cb == nullptr -> Connected() = true

    RegisterEventHandler();
    LookupAll();
}

bool UbusManager::Connect()
{
    UloopSignalGuard guard; // ubus_connect_ctx() indirectly calls ubus_init()
    VerifyOrReturnValue(CheckAndLog(ubus_connect_ctx(&Context(), mUbusSocketPath), "ubus_connect_ctx"), false);
    Context().connection_lost = [](ubus_context * ctx) { static_cast<UbusManager *>(ctx)->HandleConnectionLost(); };
    ubus_add_uloop(&Context());
    return true;
}

void UbusManager::Register(UbusWatch & watch)
{
    VerifyOrDie(mInitialized);
    mWatches.PushBack(&watch);
    if (Connected())
    {
        RegisterEventHandler();
        Lookup(watch);
    }
}

void UbusManager::Unregister(UbusWatch & watch)
{
    VerifyOrDie(mInitialized);
    mWatches.Remove(&watch);
    watch.mID = 0;

    if (Connected())
    {
        Unsubscribe(watch);
        UnregisterEventHandler();
    }

    if (watch.SubscriberActive())
    {
        // Failure to unsubscribe is problematic, because it means ubus_subscriber.obj
        // is still internaly referenced by the ubus_context. Force a cleanup of the context.
        HandleConnectionLost();
    }
}

void UbusManager::RegisterEventHandler()
{
    VerifyOrReturn(!EventHandlerActive() && !mWatches.Empty());
    VerifyOrReturn(
        CheckAndLog(ubus_register_event_handler(&Context(), &EventHandler(), "ubus.object.*"), "ubus_register_event_handler"));
    EventHandler().cb = [](ubus_context * ctx, ubus_event_handler * ev, const char * type, blob_attr * msg) {
        static_cast<UbusManager *>(ev)->HandleEvent(type, msg); // downcast from base class pointer
    };
}

void UbusManager::UnregisterEventHandler()
{
    VerifyOrReturn(EventHandlerActive() && mWatches.Empty());
    VerifyOrReturn(CheckAndLog(ubus_unregister_event_handler(&Context(), &EventHandler()), "ubus_unregister_event_handler"));
    ResetEventHandler(); // only if successfully unregistered
}

void UbusManager::Lookup(UbusWatch & watch)
{
    uint32_t id;
    int status = ubus_lookup_id(&Context(), watch.mName, &id);
    VerifyOrReturn(status != UBUS_STATUS_NOT_FOUND); // don't log "not found"
    VerifyOrReturn(CheckAndLog(status, "ubus_lookup_id"));
    Resolved(watch, id);
}

void UbusManager::LookupAll()
{
    VerifyOrReturn(!mWatches.Empty());
    ubus_lookup(
        &Context(), nullptr,
        [](ubus_context * ctx, ubus_object_data * data, void * priv) {
            auto * self = static_cast<UbusManager *>(ctx); // downcast from base class
            for (auto & watch : self->mWatches)
            {
                if (!strcmp(data->path, watch.mName))
                {
                    self->Resolved(watch, data->id);
                }
            }
        },
        nullptr);
}

void UbusManager::Subscribe(UbusWatch & watch)
{
    if (!watch.SubscriberActive())
    {
        VerifyOrReturn(CheckAndLog(ubus_register_subscriber(&Context(), &watch.Subscriber()), "ubus_register_subscriber"));
        watch.Subscriber().cb = [](ubus_context * ctx, ubus_object * sub, ubus_request_data * req, const char * type,
                                   struct blob_attr * msg) {
            auto * receiver = static_cast<UbusWatch *>(container_of(sub, ubus_subscriber, obj)); // downcast from base class
            static_cast<UbusManager *>(ctx)->HandleNotification(*receiver, req, type, msg);
            return 0;
        };
    }
    CheckAndLog(ubus_subscribe(&Context(), &watch.Subscriber(), watch.mID), "ubus_subscribe");
}

void UbusManager::Unsubscribe(UbusWatch & watch)
{
    VerifyOrReturn(watch.SubscriberActive());
    // Unregistering the subscriber also cleans up subscriptions.
    CheckAndLog(ubus_unregister_subscriber(&Context(), &watch.Subscriber()), "ubus_unregister_subscriber");
}

void UbusManager::Resolved(UbusWatch & watch, uint32_t id)
{
    watch.mID = id;
    ChipLogDetail(DeviceLayer, "Resolved ubus object %s (0x%08x)", watch.mName, id);

    if (watch.mNotificationCb != nullptr)
    {
        Subscribe(watch);
    }

    VerifyOrReturn(watch.mResolvedCb != nullptr);
    watch.mResolvedCb(watch, watch.mAppState);
}

void UbusManager::Lost(UbusWatch & watch)
{
    watch.mID = 0;
    VerifyOrReturn(watch.mLostCb != nullptr);
    watch.mLostCb(watch, watch.mAppState);
}

void UbusManager::HandleEvent(const char * type, blob_attr * msg)
{
    bool add = !strcmp(type, "ubus.object.add");
    VerifyOrReturn(add || !strcmp(type, "ubus.object.remove"));

    BlobMsgRequiredField<uint32_t, CHIP_CTST("id")> id;
    BlobMsgRequiredField<const char *, CHIP_CTST("path")> path;
    VerifyOrReturn(BlobMsgParse(msg, id, path) && id != 0);

    for (auto & watch : mWatches)
    {
        if (!strcmp(path, watch.mName))
        {
            if (add)
            {
                Resolved(watch, id);
            }
            else if (id == watch.mID)
            {
                // The subscription (if any) was cleaned up by ubusd; keep the subscriber.
                ChipLogDetail(DeviceLayer, "Lost ubus object %s (0x%08x) - removed", path.value(), id.value());
                Lost(watch);
            }
        }
    }
}

void UbusManager::HandleNotification(UbusWatch & watch, ubus_request_data * req, const char * type, blob_attr * msg)
{
    ChipLogDetail(DeviceLayer, "Received ubus notification %s.%s", watch.mName, type);
    VerifyOrReturn(watch.mNotificationCb);
    watch.mNotificationCb(watch, watch.mAppState, req, type, msg);
}

} // namespace ubus
} // namespace chip
