/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {

using namespace chip::DeviceLayer;

namespace app {
namespace server {

class TestServer : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        Platform::MemoryShutdown();
        PlatformMgr().Shutdown();
    }
};

class TestEventHandler
{
public:
    ChipDeviceEvent mEvent{};

    static void EventHandler(const ChipDeviceEvent * event, intptr_t arg)
    {
        reinterpret_cast<TestEventHandler *>(arg)->mEvent = *event;
    }
};

TEST_F(TestServer, TestFactoryResetEvent)
{
    TestEventHandler handler;
    PlatformMgr().AddEventHandler(TestEventHandler::EventHandler, reinterpret_cast<intptr_t>(&handler));

    Server::GetInstance().ScheduleFactoryReset();

    PlatformMgr().ScheduleWork([](intptr_t) -> void { PlatformMgr().StopEventLoopTask(); });
    PlatformMgr().RunEventLoop();

    EXPECT_EQ(handler.mEvent.Type, DeviceEventType::kFactoryReset);

    PlatformMgr().RemoveEventHandler(TestEventHandler::EventHandler, reinterpret_cast<intptr_t>(&handler));
}

} // namespace server
} // namespace app
} // namespace chip
