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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/IntrusiveList.h>

#include <optional>
#include <signal.h>

extern "C" {
#include <libubus.h>
#undef fallthrough // from libubox/utils.h, interferes with the standard attribute
}

namespace chip {
namespace ubus {

class UbusWatch;

// Maintains a connection to ubus and makes it available to other components.
class UbusManager : private ubus_context, private uloop_timeout, private ubus_event_handler
{
public:
    // Creates an UbusManager. A non-null socket path can be specified for testing purposes.
    UbusManager(const char * ubusSocketPath = nullptr) :
        ubus_context{}, uloop_timeout{}, ubus_event_handler{}, mUbusSocketPath(ubusSocketPath)
    {}
    ~UbusManager() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    ubus_context & Context() { return *this; }
    void Register(UbusWatch & watch);
    void Unregister(UbusWatch & watch);

private:
    const char * const mUbusSocketPath;
    IntrusiveList<UbusWatch> mWatches{};
    bool mInitialized = false;

    ///// Connection management

    uloop_timeout & ReconnectTimer() { return *this; }
    bool Connected() { return ReconnectTimer().cb == nullptr; }
    void HandleConnectionLost();
    void AttemptReconnect();
    bool Connect();

    ///// Watch / subcription management

    ubus_event_handler & EventHandler() { return *this; }
    bool EventHandlerActive() { return EventHandler().cb != nullptr; }
    void ResetEventHandler() { memset(&EventHandler(), 0, sizeof(EventHandler())); }
    void RegisterEventHandler();
    void UnregisterEventHandler();

    void Lookup(UbusWatch & watch);
    void LookupAll();
    void Subscribe(UbusWatch & watch);
    void Unsubscribe(UbusWatch & watch);
    void Resolved(UbusWatch & watch, uint32_t id);
    void Lost(UbusWatch & watch);
    void HandleEvent(const char * type, blob_attr * msg);
    void HandleNotification(UbusWatch & watch, ubus_request_data * req, const char * type, blob_attr * msg);
};

// Represents a named ubus object being tracked, and optionally subscribed to.
class UbusWatch final : public IntrusiveListNodeBase<>, private ubus_subscriber
{
public:
    using ResolvedCallback     = void (*)(UbusWatch & watch, void * appState);
    using LostCallback         = void (*)(UbusWatch & watch, void * appState);
    using NotificationCallback = void (*)(UbusWatch & watch, void * appState, ubus_request_data * req, const char * notification,
                                          blob_attr * msg);

    UbusWatch(const char * name, void * appState = nullptr) : ubus_subscriber{}, mAppState(appState), mName(name) {}

    const char * Name() { return mName; }
    UbusWatch & SetName(const char * name)
    {
        VerifyOrDie(!IsInList());
        mName = name;
        return *this;
    }

    // Sets the callback to call when the object becomes available.
    UbusWatch & SetResolvedCallback(ResolvedCallback resolvedCb)
    {
        VerifyOrDie(!IsInList());
        mResolvedCb = resolvedCb;
        return *this;
    }

    // Sets the callback to call when the object becomes unavailable.
    UbusWatch & SetLostCallback(LostCallback lostCb)
    {
        VerifyOrDie(!IsInList());
        mLostCb = lostCb;
        return *this;
    }

    UbusWatch & SetNotificationCallback(NotificationCallback notificationCb)
    {
        VerifyOrDie(!IsInList());
        mNotificationCb = notificationCb;
        return *this;
    }

    // The resolved object id, or 0 if not currently resolved.
    uint32_t ObjectID() const { return mID; }
    bool Resolved() const { return ObjectID() != 0; }

private:
    friend class UbusManager;

    ubus_subscriber & Subscriber() { return *this; }
    bool SubscriberActive() { return Subscriber().cb != nullptr; }
    void ResetSubscriber() { memset(&Subscriber(), 0, sizeof(Subscriber())); }
    void Reset() { mID = 0; }

    void * const mAppState;
    const char * mName;
    ResolvedCallback mResolvedCb;
    LostCallback mLostCb;
    NotificationCallback mNotificationCb;

    uint32_t mID = 0;
};

} // namespace ubus
} // namespace chip
