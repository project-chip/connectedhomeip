/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/address_resolve/AddressResolve.h>
#include <lib/address_resolve/NodeAddressCache.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;
using namespace chip::AddressResolve;

namespace {

TEST(TestNodeAddressCache, CacheBasicOperations)
{
    NodeAddressCache cache;
    ResolveResult result;
    ResolveResult retrievedResult;

    // Test empty cache
    PeerId peerId(1, 1);
    EXPECT_EQ(cache.GetCacheSize(), 0u);
    EXPECT_EQ(cache.Lookup(peerId, retrievedResult), false);

    // Test caching a node
    Inet::IPAddress ipAddress;
    chip::Inet::IPAddress::FromString("192.168.1.1", ipAddress);
    result.address.SetIPAddress(ipAddress);
    result.address.SetPort(5540);
    cache.CacheNode(peerId, result);
    EXPECT_EQ(cache.GetCacheSize(), 1u);

    // Test retrieving cached node
    EXPECT_EQ(cache.Lookup(peerId, retrievedResult), true);
    EXPECT_EQ(retrievedResult.address.GetIPAddress(), result.address.GetIPAddress());
    EXPECT_EQ(retrievedResult.address.GetPort(), result.address.GetPort());

    // Test removing cached node
    EXPECT_EQ(cache.RemoveCachedNodeAddress(peerId), CHIP_NO_ERROR);
    EXPECT_EQ(cache.Lookup(peerId, retrievedResult), false);
}

TEST(TestNodeAddressCache, CacheUpdate)
{
    NodeAddressCache cache;
    ResolveResult result1, result2, retrievedResult;

    // Cache initial result
    PeerId peerId(1, 1);
    Inet::IPAddress ipAddress1;
    chip::Inet::IPAddress::FromString("192.168.1.1", ipAddress1);
    result1.address.SetIPAddress(ipAddress1);
    result1.address.SetPort(5540);
    cache.CacheNode(peerId, result1);
    EXPECT_EQ(cache.GetCacheSize(), 1u);

    // Update with new result
    Inet::IPAddress ipAddress2;
    chip::Inet::IPAddress::FromString("192.168.1.2", ipAddress2);
    result2.address.SetIPAddress(ipAddress2);
    result2.address.SetPort(5541);
    cache.CacheNode(peerId, result2);
    EXPECT_EQ(cache.GetCacheSize(), 1u); // Size should remain 1

    // Verify updated result
    EXPECT_EQ(cache.Lookup(peerId, retrievedResult), true);
    EXPECT_EQ(retrievedResult.address.GetIPAddress(), result2.address.GetIPAddress());
    EXPECT_EQ(retrievedResult.address.GetPort(), result2.address.GetPort());
}

TEST(TestNodeAddressCache, CacheFIFOReplacement)
{
    NodeAddressCache cache;
    ResolveResult result, retrievedResult;

    // Fill cache to capacity
    for (size_t i = 0; i < NodeAddressCache::kMaxCacheSize; ++i)
    {
        PeerId peerId(1, i + 1);
        Inet::IPAddress ipAddress;
        chip::Inet::IPAddress::FromString("192.168.1.1", ipAddress);
        result.address.SetIPAddress(ipAddress);
        result.address.SetPort((uint16_t) (5540 + i));
        cache.CacheNode(peerId, result);
    }
    EXPECT_EQ(cache.GetCacheSize(), NodeAddressCache::kMaxCacheSize);

    // Add one more entry (should replace oldest)
    PeerId newPeerId(1, 100);
    result.address.SetPort(9999);
    cache.CacheNode(newPeerId, result);
    EXPECT_EQ(cache.GetCacheSize(), NodeAddressCache::kMaxCacheSize);

    // First entry should be gone
    PeerId firstPeerId(1, 1);
    EXPECT_EQ(cache.Lookup(firstPeerId, retrievedResult), false);

    // New entry should be present
    EXPECT_EQ(cache.Lookup(newPeerId, retrievedResult), true);
    EXPECT_EQ(retrievedResult.address.GetPort(), 9999);
}

TEST(TestNodeAddressCache, CacheClear)
{
    NodeAddressCache cache;
    ResolveResult result, retrievedResult;

    // Add some entries
    Inet::IPAddress ipAddress;
    chip::Inet::IPAddress::FromString("192.168.1.1", ipAddress);
    result.address.SetIPAddress(ipAddress);
    for (size_t i = 0; i < 5; ++i)
    {
        PeerId peerId(1, i + 1);
        result.address.SetPort((uint16_t) (5540 + i));
        cache.CacheNode(peerId, result);
    }
    EXPECT_EQ(cache.GetCacheSize(), 5u);

    // Clear cache
    cache.Clear();
    EXPECT_EQ(cache.GetCacheSize(), 0u);

    // Verify all entries are gone
    for (size_t i = 0; i < 5; ++i)
    {
        PeerId peerId(1, i + 1);
        EXPECT_EQ(cache.Lookup(peerId, retrievedResult), false);
    }
}

TEST(TestNodeAddressCache, RemoveNonExistentNode)
{
    NodeAddressCache cache;
    PeerId nonExistentPeerId(1, 999);
    EXPECT_EQ(cache.RemoveCachedNodeAddress(nonExistentPeerId), CHIP_ERROR_KEY_NOT_FOUND);
}

} // namespace
