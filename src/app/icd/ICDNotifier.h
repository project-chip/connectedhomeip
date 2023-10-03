/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/AppConfig.h>
#include <cstdint>
#include <cstring>

#ifndef ICD_MAX_NOTIFICATION_SUBSCRIBERS
#define ICD_MAX_NOTIFICATION_SUBSCRIBERS 1
#endif

class ICDNotify;

namespace chip {
namespace app {

class ICDNotify
{
public:
    enum class KeepActiveFlags : uint8_t
    {
        kCommissioningWindowOpen = 0x01,
        kFailSafeArmed           = 0x02,
        kExchangeContextOpen     = 0x03,
    };

    virtual ~ICDNotify() {}
    virtual void NotifyNetworkActivity()                              = 0;
    virtual void KeepActiveRequest(KeepActiveFlags request, bool set) = 0;
};

class ICDNotifier
{
public:
    ICDNotifier() { memset(subscribers, 0, sizeof(subscribers)); }
    ~ICDNotifier() { memset(subscribers, 0, sizeof(subscribers)); }

    void Subscribe(ICDNotify * subscriber)
    {
        for (auto & sub : subscribers)
        {
            if (sub == nullptr)
            {
                sub = subscriber;
                break;
            }
        }
    }

    void Unsubscribe(ICDNotify * subscriber)
    {
        for (auto & sub : subscribers)
        {
            if (sub == subscriber)
            {
                sub = nullptr;
                break;
            }
        }
    }

    void NetworkActivity() const
    {
        for (auto subscriber : subscribers)
        {
            if (subscriber != nullptr)
            {
                subscriber->NotifyNetworkActivity();
            }
        }
    }

    void ActiveRequest(ICDNotify::KeepActiveFlags request, bool set) const
    {
        for (auto subscriber : subscribers)
        {
            if (subscriber != nullptr)
            {
                subscriber->KeepActiveRequest(request, set);
            }
        }
    }

private:
    ICDNotify * subscribers[ICD_MAX_NOTIFICATION_SUBSCRIBERS];
};

ICDNotifier * GetICDNotifier();

} // namespace app
} // namespace chip
