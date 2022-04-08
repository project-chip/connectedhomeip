/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <bitset>
#include <cstddef>
#include <type_traits>

#include <lib/core/CHIPError.h>

namespace chip {
namespace Retransmit {

/**
 * Handles lifetimes of payloads.
 *
 * E.g. for buffers, handles reference counting.
 */
template <typename PayloadType>
struct Lifetime
{
    static PayloadType Acquire(PayloadType & payload);
    static void Release(PayloadType & payload);
};

/**
 * This class maintains a cache of data that is sufficient to retransmit.
 *
 * Typical use is to keep track of unacknowledged packets and resend them
 * as needed.
 *
 * @tparam KeyType the key to identify a single message
 * @tparam PayloadType the type of payload to cache for the given peer address
 * @tparam N size of the available cache
 *
 * PayloadType MUST provide a way to reference count, as the data will
 * preserved until the cache is freed:
 *
 *    PayloadType chip::Retransmit::Acquire(PayloadType&);
 *    chip::Retransmit::Release(PayloadType&);
 */
template <typename KeyType, typename PayloadType, size_t N>
class Cache
{
public:
    Cache() {}
    Cache(const Cache &) = delete;
    Cache & operator=(const Cache &) = delete;

    ~Cache()
    {
        for (size_t i = 0; i < N; i++)
        {
            if (mInUse.test(i))
            {
                Lifetime<PayloadType>::Release(mEntries[i].payload);
            }
        }
    }

    /**
     * Add a payload to the cache.
     */
    CHIP_ERROR Add(const KeyType & key, PayloadType & payload)
    {
        if (mInUse.count() >= N)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        for (size_t i = 0; i < N; i++)
        {
            if (!mInUse.test(i))
            {
                mInUse.set(i);
                mEntries[i].key     = key;
                mEntries[i].payload = Lifetime<PayloadType>::Acquire(payload);
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

    /**
     * Remove a payload from the cache given the key.
     */
    CHIP_ERROR Remove(const KeyType & key)
    {
        for (size_t i = 0; i < N; i++)
        {
            if (mInUse.test(i) && (mEntries[i].key == key))
            {
                mInUse.reset(i);
                Lifetime<PayloadType>::Release(mEntries[i].payload);
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    /**
     * Remove any matching payloads. Used for mass removal, e.g. when a connection
     * is closed, relevant payloads need/can be cleared for the entire connection.
     *
     * @tparam Matcher is a generic matcher object defining a bool Matches method.
     */
    template <typename Matcher>
    void RemoveMatching(const Matcher & matcher)
    {
        for (size_t i = 0; i < N; i++)
        {
            if (mInUse.test(i) && matcher.Matches(mEntries[i].key))
            {
                mInUse.reset(i);
                Lifetime<PayloadType>::Release(mEntries[i].payload);
            }
        }
    }

    /**
     * Search for a specific entry within the cache.
     *
     * @tparam Matcher is a generic macher object defining a bool Maches method.
     *
     * @param matcher the entry to find
     * @param key - out set the key if found
     * @param payload - the payload if found
     *
     * Key and payload are only valid as long as no remove methods
     * are called on the class.
     */
    template <typename Matcher>
    bool Find(const Matcher & matcher, const KeyType ** key, const PayloadType ** payload)
    {
        *key     = nullptr;
        *payload = nullptr;

        for (size_t i = 0; i < N; i++)
        {
            if (mInUse.test(i) && matcher.Matches(mEntries[i].key))
            {
                *key     = &mEntries[i].key;
                *payload = &mEntries[i].payload;
                return true;
            }
        }
        return false;
    }

private:
    struct Entry
    {
        KeyType key;
        PayloadType payload;
    };

    Entry mEntries[N];     // payload entries
    std::bitset<N> mInUse; // compact 'in use' marker for payloads
};                         // namespace Retransmit

} // namespace Retransmit
} // namespace chip
