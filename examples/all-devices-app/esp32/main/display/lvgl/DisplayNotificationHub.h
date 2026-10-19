/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/AttributeChangeListener.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <lib/core/DataModelTypes.h>
#include <lvgl.h>
#include <vector>

namespace chip::app {

/**
 * Bridges Matter DataModel attribute change events to LVGL display widgets.
 *
 * Runs as a singleton AttributeChangeListener registered with the Server's
 * DataModel::Provider. When attribute changes occur on the CHIP thread,
 * DisplayNotificationHub acquires the display lock and dispatches updates
 * to registered callbacks.
 */
class DisplayNotificationHub : public DataModel::AttributeChangeListener
{
public:
    using AttributeCallback = std::function<void(const ConcreteAttributePath & path)>;

    static DisplayNotificationHub & Instance();

    /// Registers this hub as an AttributeChangeListener with the Server DataModelProvider.
    /// Must be called after the server data model provider is initialized.
    void Init();

    /// Subscribes a lambda callback for attribute changes matching (endpoint, cluster).
    /// If owner (e.g. card/widget) is non-null, an LV_EVENT_DELETE handler is
    /// automatically attached to owner so the subscription unregisters when the widget is destroyed.
    ///
    /// Must be called with the display lock held (which is already held during screen rendering).
    void Subscribe(lv_obj_t * owner, EndpointId endpoint, ClusterId cluster, AttributeCallback callback);

    /// Explicitly unregisters all subscriptions belonging to owner.
    /// Must be called with the display lock held.
    void Unsubscribe(lv_obj_t * owner);

    // DataModel::AttributeChangeListener implementation (invoked on CHIP thread)
    void OnAttributeChanged(const ConcreteAttributePath & path, DataModel::AttributeChangeType type) override;

private:
    struct Subscription
    {
        uint32_t id;
        lv_obj_t * owner;
        EndpointId endpoint;
        ClusterId cluster;
        AttributeCallback callback;
    };

    bool mRegistered                       = false;
    uint32_t mNextId                       = 1;
    std::atomic<size_t> mSubscriptionCount = 0;
    std::vector<Subscription> mSubscriptions;
};

} // namespace chip::app
