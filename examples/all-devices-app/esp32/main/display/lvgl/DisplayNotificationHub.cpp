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

#include "DisplayNotificationHub.h"

#include <algorithm>
#include <app/InteractionModelEngine.h>
#include <bsp/esp-bsp.h>
#include <esp_log.h>

namespace chip::app {

namespace {
const char TAG[]                = "DisplayHub";
constexpr uint32_t kWaitForever = 0;
} // namespace

DisplayNotificationHub & DisplayNotificationHub::Instance()
{
    static DisplayNotificationHub sInstance;
    return sInstance;
}

void DisplayNotificationHub::Init()
{
    if (mRegistered)
    {
        return;
    }

    auto * imEngine = InteractionModelEngine::GetInstance();
    if (imEngine == nullptr)
    {
        ESP_LOGW(TAG, "InteractionModelEngine not ready yet");
        return;
    }

    auto * provider = imEngine->GetDataModelProvider();
    if (provider == nullptr)
    {
        ESP_LOGW(TAG, "Server DataModelProvider not ready yet");
        return;
    }

    provider->RegisterAttributeChangeListener(*this);
    mRegistered = true;
    ESP_LOGI(TAG, "DisplayNotificationHub registered with DataModelProvider");
}

void DisplayNotificationHub::Subscribe(lv_obj_t * owner, EndpointId endpoint, ClusterId cluster, AttributeCallback callback)
{
    bool alreadyHooked = false;
    if (owner != nullptr)
    {
        for (const auto & sub : mSubscriptions)
        {
            if (sub.owner == owner)
            {
                alreadyHooked = true;
                break;
            }
        }
    }

    mSubscriptions.push_back({ mNextId++, owner, endpoint, cluster, std::move(callback) });
    mSubscriptionCount.store(mSubscriptions.size(), std::memory_order_release);

    if (owner != nullptr && !alreadyHooked)
    {
        lv_obj_add_event_cb(
            owner,
            [](lv_event_t * e) {
                auto * self = static_cast<DisplayNotificationHub *>(lv_event_get_user_data(e));
                self->Unsubscribe(lv_event_get_target_obj(e));
            },
            LV_EVENT_DELETE, this);
    }
}

void DisplayNotificationHub::Unsubscribe(lv_obj_t * owner)
{
    if (owner == nullptr)
    {
        return;
    }

    mSubscriptions.erase(
        std::remove_if(mSubscriptions.begin(), mSubscriptions.end(), [owner](const Subscription & s) { return s.owner == owner; }),
        mSubscriptions.end());
    mSubscriptionCount.store(mSubscriptions.size(), std::memory_order_release);
}

void DisplayNotificationHub::OnAttributeChanged(const ConcreteAttributePath & path, DataModel::AttributeChangeType type)
{
    if (mSubscriptionCount.load(std::memory_order_acquire) == 0)
    {
        return;
    }

    if (!bsp_display_lock(kWaitForever))
    {
        ESP_LOGE(TAG, "Could not acquire display lock for attribute change");
        return;
    }

    std::vector<uint32_t> matchingIds;
    for (const auto & sub : mSubscriptions)
    {
        if ((sub.endpoint == kInvalidEndpointId || sub.endpoint == path.mEndpointId) &&
            (sub.cluster == kInvalidClusterId || sub.cluster == path.mClusterId))
        {
            matchingIds.push_back(sub.id);
        }
    }

    for (uint32_t id : matchingIds)
    {
        auto it = std::find_if(mSubscriptions.begin(), mSubscriptions.end(), [id](const Subscription & s) { return s.id == id; });
        if (it != mSubscriptions.end())
        {
            AttributeCallback cb = it->callback;
            cb(path);
        }
    }

    bsp_display_unlock();
}

} // namespace chip::app
