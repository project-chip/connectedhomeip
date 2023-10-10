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

#include <app/icd/ICDNotifier.h>
#include <cstdint>
#include <cstring>

namespace chip {
namespace app {

ICDNotifier ICDNotifier::sICDNotifier;

ICDNotifier::~ICDNotifier()
{
    memset(mSubscribers, 0, sizeof(mSubscribers));
}

CHIP_ERROR ICDNotifier::Subscribe(ICDListener * subscriber)
{
    CHIP_ERROR err = CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    for (auto & sub : mSubscribers)
    {
        if (sub == nullptr)
        {
            sub = subscriber;
            err = CHIP_NO_ERROR;
            break;
        }
    }
    return err;
}

void ICDNotifier::Unsubscribe(ICDListener * subscriber)
{
    for (auto & sub : mSubscribers)
    {
        if (sub == subscriber)
        {
            sub = nullptr;
            break;
        }
    }
}

void ICDNotifier::BroadcastNetworkActivityNotification()
{
    for (auto subscriber : mSubscribers)
    {
        if (subscriber != nullptr)
        {
            subscriber->OnNetworkActivity();
        }
    }
}

void ICDNotifier::BroadcastActiveRequestNotification(ICDListener::KeepActiveFlags request)
{
    for (auto subscriber : mSubscribers)
    {
        if (subscriber != nullptr)
        {
            subscriber->OnKeepActiveRequest(request);
        }
    }
}

void ICDNotifier::BroadcastActiveRequestWithdrawal(ICDListener::KeepActiveFlags request)
{
    for (auto subscriber : mSubscribers)
    {
        if (subscriber != nullptr)
        {
            subscriber->OnActiveRequestWithdrawal(request);
        }
    }
}

} // namespace app
} // namespace chip
