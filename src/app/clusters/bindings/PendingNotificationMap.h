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
#include <lib/core/DataModelTypes.h>

namespace chip {

struct PendingNotificationEntry
{
public:
    uint8_t mBindingEntryId;
    void * mContext;
};

// The pool for all the pending comands.
class PendingNotificationMap
{
public:
    static constexpr uint8_t kMaxPendingNotifications = EMBER_BINDING_TABLE_SIZE;

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
            if (mIndex == mMap->mNumEntries)
            {
                // end of iteration
                mIndex = -1;
            }
            mIndex = static_cast<int16_t>(mIndex % kMaxPendingNotifications);
            return *this;
        }

        bool operator!=(const Iterator & rhs) { return mIndex != rhs.mIndex; }

    private:
        PendingNotificationMap * mMap;
        int16_t mIndex;
    };

    Iterator begin() { return Iterator(this, mNumEntries > 0 ? mNextToOverride : -1); }

    Iterator end() { return Iterator(this, -1); }

    int16_t FindLRUBindingEntryIndex();

    void AddPendingNotification(uint8_t bindingEntryId, void * context);

    void RemoveEntry(uint8_t bindingEntryId);

    void RemoveAllEntriesForNode(FabricIndex fabric, NodeId node);

private:
    uint8_t mPendingBindingEntries[kMaxPendingNotifications];
    void * mPendingContexts[kMaxPendingNotifications];

    uint8_t mNumEntries     = 0;
    uint8_t mNextToOverride = 0;
};

void TestPendingNotificationMap();

} // namespace chip
