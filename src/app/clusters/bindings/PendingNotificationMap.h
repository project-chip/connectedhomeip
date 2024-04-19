/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/util/binding-table.h>
#include <app/util/config.h>
#include <credentials/FabricTable.h>
#include <lib/core/DataModelTypes.h>

namespace chip {

/**
 * Application callback function when a context used in PendingNotificationEntry will not be needed and should be
 * released.
 */
using PendingNotificationContextReleaseHandler = void (*)(void * context);

class PendingNotificationContext
{
public:
    PendingNotificationContext(void * context, PendingNotificationContextReleaseHandler contextReleaseHandler) :
        mContext(context), mPendingNotificationContextReleaseHandler(contextReleaseHandler)
    {}
    void * GetContext() { return mContext; };
    uint32_t GetConsumersNumber() { return mConsumersNumber; }
    void IncrementConsumersNumber() { mConsumersNumber++; }
    void DecrementConsumersNumber()
    {
        VerifyOrDie(mConsumersNumber > 0);
        if (--mConsumersNumber == 0)
        {
            // Release the context only if there is no pending notification pointing to us.
            if (mPendingNotificationContextReleaseHandler != nullptr)
            {
                mPendingNotificationContextReleaseHandler(mContext);
            }
            Platform::Delete(this);
        }
    }

private:
    void * mContext;
    uint32_t mConsumersNumber = 0;
    PendingNotificationContextReleaseHandler mPendingNotificationContextReleaseHandler;
};

struct PendingNotificationEntry
{
public:
    uint8_t mBindingEntryId;
    PendingNotificationContext * mContext;
};

// The pool for all the pending comands.
class PendingNotificationMap
{
public:
    static constexpr uint8_t kMaxPendingNotifications = MATTER_BINDING_TABLE_SIZE;

    friend class Iterator;

    class Iterator
    {
    public:
        Iterator(PendingNotificationMap * map, int16_t index) : mMap(map), mIndex(index) {}

        PendingNotificationEntry operator*()
        {
            return PendingNotificationEntry{ mMap->mPendingBindingEntries[mIndex], mMap->mPendingContexts[mIndex] };
        }

        Iterator operator++()
        {
            mIndex++;
            return *this;
        }

        bool operator!=(const Iterator & rhs) const { return mIndex != rhs.mIndex; }

        bool operator==(const Iterator & rhs) const { return mIndex == rhs.mIndex; }

    private:
        PendingNotificationMap * mMap;
        int16_t mIndex;
    };

    Iterator begin() { return Iterator(this, 0); }

    Iterator end() { return Iterator(this, mNumEntries); }

    CHIP_ERROR FindLRUConnectPeer(ScopedNodeId & nodeId);

    CHIP_ERROR AddPendingNotification(uint8_t bindingEntryId, PendingNotificationContext * context);

    void RemoveEntry(uint8_t bindingEntryId);

    void RemoveAllEntriesForNode(const ScopedNodeId & nodeId);

    void RemoveAllEntriesForFabric(FabricIndex fabric);

    void RegisterPendingNotificationContextReleaseHandler(PendingNotificationContextReleaseHandler handler)
    {
        mPendingNotificationContextReleaseHandler = handler;
    }

    PendingNotificationContext * NewPendingNotificationContext(void * context)
    {
        return Platform::New<PendingNotificationContext>(context, mPendingNotificationContextReleaseHandler);
    };

private:
    uint8_t mPendingBindingEntries[kMaxPendingNotifications];
    PendingNotificationContext * mPendingContexts[kMaxPendingNotifications];
    PendingNotificationContextReleaseHandler mPendingNotificationContextReleaseHandler;

    uint8_t mNumEntries = 0;
};

} // namespace chip
