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
#ifndef RETRANSMIT_MESSAGE_CACHE_H_
#define RETRANSMIT_MESSAGE_CACHE_H_

#include <bitset>
#include <cstddef>

#include <transport/PeerAddress.h>

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
 * @tparam PayloadType the type of payload to cache for the given peer address
 * @tparam N size of the available cache
 *
 * PayloadType MUST provide a way to reference count, as the data will
 * preserved until the cache is freed:
 *
 *    PayloadType chip::Retransmit::Aquire(PayloadType&);
 *    chip::Retransmit::Release(PayloadType&);
 */
template <typename PayloadType, size_t N>
class PeerAddressedCache
{
public:
    PeerAddressedCache() {}
    PeerAddressedCache(const PeerAddressedCache &) = delete;
    PeerAddressedCache & operator=(const PeerAddressedCache &) = delete;

    ~PeerAddressedCache()
    {
        for (unsigned i = 0; i < N; i++)
        {
            if (mPayloadInUse.test(i))
            {
                Lifetime<PayloadType>::Release(mPayloads[i].payload);
            }
        }
    }

private:
    struct Entry
    {
        Transport::PeerAddress peerAddress;
        PayloadType payload;
    };

    Entry mPayloads[N];           // payload entries
    std::bitset<N> mPayloadInUse; // compact 'in use' marker for payloads
};

} // namespace Retransmit
} // namespace chip

#endif // RETRANSMIT_MESSAGE_CACHE_H_