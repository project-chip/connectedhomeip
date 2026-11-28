/*
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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

#include <app/clusters/bindings/binding-table.h>
#include <credentials/FabricTable.h>
#include <lib/core/DataModelTypes.h>
#include <system/SystemLayer.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {

/**
 * Application callback function when a context used in PendingNotificationEntry will not be needed and should be
 * released.
 */
using PendingNotificationContextReleaseHandler = void (*)(void * context);

using PendingCommandFailureHandler = void (*)(const TableEntry & binding, void * context, CHIP_ERROR error);

class PendingNotificationMap;

class PendingNotificationContext
{
public:
    PendingNotificationContext(void * context, PendingNotificationContextReleaseHandler contextReleaseHandler,
                               PendingCommandFailureHandler failureHandler = nullptr) :
        mFailureHandler(failureHandler),
        mContext(context), mPendingNotificationContextReleaseHandler(contextReleaseHandler)
    {}
    ~PendingNotificationContext()
    {
        if (mSystemLayer != nullptr)
        {
            mSystemLayer->CancelTimer(HandleTimeout, this);
        }
    }
    bool HasExpired() const { return IsCommand() && System::SystemClock().GetMonotonicTimestamp() >= mDeadline; }
    bool IsCommand() const { return mFailureHandler != nullptr; }
    void Failed(const TableEntry & binding, CHIP_ERROR error)
    {
        if (mFailureHandler != nullptr && error != CHIP_NO_ERROR)
        {
            mFailureHandler(binding, mContext, error);
        }
    }
    CHIP_ERROR StartTimer(PendingNotificationMap & map, System::Layer & layer, System::Clock::Timeout timeout)
    {
        mDeadline    = System::SystemClock().GetMonotonicTimestamp() + timeout;
        mMap         = &map;
        mSystemLayer = &layer;
        return layer.StartTimer(timeout, HandleTimeout, this);
    }
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
    System::Clock::Timestamp mDeadline = System::Clock::kZero;
    PendingNotificationMap * mMap      = nullptr;
    System::Layer * mSystemLayer       = nullptr;
    static void HandleTimeout(System::Layer *, void * state);
    PendingCommandFailureHandler mFailureHandler = nullptr;
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

// The pool for all the pending commands.
class PendingNotificationMap
{
public:
    static constexpr uint8_t kMaxPendingNotifications = Table::kMaxBindingEntries;

    PendingNotificationMap(Table & bindingTable) : mBindingTable(bindingTable) {}
    ~PendingNotificationMap() { Shutdown(); }
    void Shutdown();
    PendingNotificationMap(const PendingNotificationMap &)             = delete;
    PendingNotificationMap & operator=(const PendingNotificationMap &) = delete;
    bool IsShuttingDown() const { return mIsShuttingDown; }

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

    /// Coalesces notifications, but returns BUSY without changing either context if either entry is a command.
    CHIP_ERROR AddPendingNotification(uint8_t bindingEntryId, PendingNotificationContext * context);

    bool HasPendingNotification(uint8_t bindingEntryId, bool commandsOnly = false) const;
    void RemoveEntry(uint8_t bindingEntryId, CHIP_ERROR error = CHIP_ERROR_CANCELLED);
    /// Bulk removals exclude entries accepted during their callbacks.
    void RemoveContext(PendingNotificationContext * context, CHIP_ERROR error);

    void RemoveAllEntriesForNode(const ScopedNodeId & nodeId);

    void RemoveAllEntriesForFabric(FabricIndex fabric);

    void RegisterPendingNotificationContextReleaseHandler(PendingNotificationContextReleaseHandler handler)
    {
        mPendingNotificationContextReleaseHandler = handler;
    }

    PendingNotificationContext * NewPendingNotificationContext(void * context,
                                                               PendingCommandFailureHandler failureHandler = nullptr)
    {
        return Platform::New<PendingNotificationContext>(context, mPendingNotificationContextReleaseHandler, failureHandler);
    };

private:
    uint8_t mPendingBindingEntries[kMaxPendingNotifications];
    PendingNotificationContext * mPendingContexts[kMaxPendingNotifications];
    uint64_t mPendingSequences[kMaxPendingNotifications];
    PendingNotificationContextReleaseHandler mPendingNotificationContextReleaseHandler = nullptr;

    Table & mBindingTable;

    uint64_t mLastSequence = 0;
    uint8_t mNumEntries    = 0;
    bool mIsShuttingDown   = false;
};

} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace chip
