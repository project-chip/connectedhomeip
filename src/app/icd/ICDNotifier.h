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
#include <stdio.h>
#include <stdlib.h>
// #include <lib/support/Pool.h>
#include <algorithm>
#include <cstdint>
#include <vector>

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
    virtual void NotifyNetworkActivity()                                = 0;
    virtual void KeepActiveRequest(KeepActiveFlags requester, bool set) = 0;
};

class ICDNotifier
{
public:
    ~ICDNotifier()
    {
        //    mSubscribersPool.ReleaseAll();
    }

    void Subscribe(ICDNotify * subscriber)
    {
        // subscriber = mSubscribersPool.CreateObject();
        subscribers.push_back(subscriber);
    }

    void Unsubscribe(ICDNotify * subscriber)
    {
        // mSubscribersPool.ReleaseObject(subscriber);
        subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
    }

    void NetworkActivity() const
    {
        // mSubscribersPool.ForEachActiveObject([](ICDNotify * subscriber) {
        //     subscriber->NotifyNetworkActivity();
        //     return Loop::Continue;
        // });
        for (ICDNotify * subscriber : subscribers)
        {
            subscriber->NotifyNetworkActivity();
        }
    }

    void ActiveRequest(ICDNotify::KeepActiveFlags requester, bool set) const
    {
        // mSubscribersPool.ForEachActiveObject([requester,set](ICDNotify * subscriber) {

        //     subscriber->KeepActiveRequest(requester, set);
        //     return Loop::Continue;
        // });

        for (ICDNotify * subscriber : subscribers)
        {
            subscriber->KeepActiveRequest(requester, set);
        }
    }

private:
    // ObjectPool<ICDNotify, ICD_MAX_NOTIFICATION_SUBSCRIBERS> mSubscribersPool;
    std::vector<ICDNotify *> subscribers;
};

ICDNotifier * GetICDNotifier();

} // namespace app
} // namespace chip
