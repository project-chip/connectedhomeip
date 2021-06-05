/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <mdns/IPCache.h>

#include <string.h>

#include <cstdint>
#include <nlunit-test.h>
#include <support/UnitTestRegistration.h>

#include <core/CHIPError.h>
#include <core/PeerId.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <system/SystemTimer.h>
#include <system/TimeSource.h>

#include <unistd.h>

using namespace chip;
using namespace chip::Mdns;

namespace {

#define TEST_IPCACHE_SIZE_SMALL 2
#define TEST_IPCACHE_TTL_MS 10000

void TestCheckOccupancy(nlTestSuite * inSuite, void * inContext)
{
    IPCache<TEST_IPCACHE_SIZE_SMALL, TEST_IPCACHE_TTL_MS> tIPCache;

    NodeId nodeId;
    FabricId fabricId;
    uint16_t port;
    Inet::InterfaceId iface = 0;

    Inet::IPAddress addr;
    Inet::IPAddress::FromString("1.0.0.1", addr);

    uint16_t i = 1;
    for (; i <= TEST_IPCACHE_SIZE_SMALL; ++i)
    {
        nodeId   = i;
        fabricId = i;
        port     = i;

        NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeId, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    }

    NL_TEST_ASSERT(inSuite, tIPCache.GetOccupancy() == TEST_IPCACHE_SIZE_SMALL);

    nodeId   = i;
    fabricId = i;
    port     = i;

    NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeId, fabricId, addr, port, iface) != CHIP_NO_ERROR);
}

void TestCheckInsert(nlTestSuite * inSuite, void * inContext)
{
    IPCache<TEST_IPCACHE_SIZE_SMALL, TEST_IPCACHE_TTL_MS> tIPCache;

    NodeId nodeIdYes = 2, nodeIdNo = 3;
    FabricId fabricId       = 2;
    uint16_t port           = 2;
    Inet::InterfaceId iface = 0;

    Inet::IPAddress addr;
    Inet::IPAddress::FromString("1.0.0.1", addr);

    NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeIdYes, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeIdYes + TEST_IPCACHE_SIZE_SMALL, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Lookup(nodeIdYes, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Lookup(nodeIdYes + TEST_IPCACHE_SIZE_SMALL, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Lookup(nodeIdNo, fabricId, addr, port, iface) != CHIP_NO_ERROR);
}

void TestCheckDelete(nlTestSuite * inSuite, void * inContext)
{
    IPCache<TEST_IPCACHE_SIZE_SMALL, TEST_IPCACHE_TTL_MS> tIPCache;

    NodeId nodeIdYes = 2, nodeIdNo = 3;
    FabricId fabricId       = 2;
    uint16_t port           = 2;
    Inet::InterfaceId iface = 0;

    Inet::IPAddress addr;
    Inet::IPAddress::FromString("1.0.0.1", addr);

    NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeIdYes, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Delete(nodeIdNo) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Delete(nodeIdYes) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tIPCache.Lookup(nodeIdYes, fabricId, addr, port, iface) != CHIP_NO_ERROR);
}

void TestCheckExpiration(nlTestSuite * inSuite, void * inContext)
{
    IPCache<TEST_IPCACHE_SIZE_SMALL, TEST_IPCACHE_TTL_MS> tIPCache;

    NodeId nodeId           = 2;
    FabricId fabricId       = 2;
    uint16_t port           = 2;
    Inet::InterfaceId iface = 0;

    Inet::IPAddress addr;
    Inet::IPAddress::FromString("1.0.0.1", addr);

    NL_TEST_ASSERT(inSuite, tIPCache.Insert(nodeId, fabricId, addr, port, iface) == CHIP_NO_ERROR);
    usleep((TEST_IPCACHE_TTL_MS + 1) * 1000);
    NL_TEST_ASSERT(inSuite, tIPCache.Lookup(nodeId, fabricId, addr, port, iface) != CHIP_NO_ERROR);
}
const nlTest sTests[] = { NL_TEST_DEF("CheckOccupancy", TestCheckOccupancy), NL_TEST_DEF("CheckInsert", TestCheckInsert),
                          NL_TEST_DEF("CheckDelete", TestCheckDelete), NL_TEST_DEF("CheckExpiration", TestCheckExpiration),
                          NL_TEST_SENTINEL() };

} // namespace

int TestIPCaache(void)
{
    nlTestSuite theSuite = { "IPCache", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestIPCaache)
