/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <binding-handler/BindingHandler.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <device/types/doorbell/Doorbell.h>
#include <lib/support/CHIPMem.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class TestBindingHandler : public ::testing::Test
{
protected:
    void SetUp() override { EXPECT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    void TearDown() override { Platform::MemoryShutdown(); }
};

class TestCustomContext : public BindingContext
{
public:
    explicit TestCustomContext(int & destructorCount) : mDestructorCount(destructorCount) {}
    ~TestCustomContext() override { mDestructorCount++; }

    ClusterId GetClusterId() const override { return 0xFFFF; }
    void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device) override {}

private:
    int & mDestructorCount;
};

TEST_F(TestBindingHandler, ContextReleaseHandlerDeletesContext)
{
    // Test that passing nullptr does not crash
    AllDevicesBoundDeviceContextReleaseHandler(nullptr);

    // Test polymorphic context destruction and memory cleanup
    int destructorCount = 0;
    auto * customContext = Platform::New<TestCustomContext>(destructorCount);
    VerifyOrDie(customContext != nullptr);

    AllDevicesBoundDeviceContextReleaseHandler(customContext);
    EXPECT_EQ(destructorCount, 1);

    // Test releasing SimpleBindingCommand
    Clusters::Chime::Commands::PlayChimeSound::Type playChimeCmd;
    auto * chimeCmd = Platform::New<SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type>>(playChimeCmd);
    VerifyOrDie(chimeCmd != nullptr);

    AllDevicesBoundDeviceContextReleaseHandler(chimeCmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesNullContextGracefully)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    // Passing nullptr context should not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, nullptr);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesNullPeerGracefully)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    chimeCmd.chimeID.SetValue(1);
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    // Passing nullptr peer_device should not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerSupportsMultipleClusters)
{
    // 1. OnOff Cluster
    Clusters::Binding::TableEntry onOffEntry;
    onOffEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    onOffEntry.local     = 1;
    onOffEntry.remote    = 2;
    onOffEntry.clusterId = Clusters::OnOff::Id;

    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> onOffCmd(toggleCmd);
    EXPECT_EQ(onOffCmd.GetClusterId(), Clusters::OnOff::Id);
    AllDevicesBoundDeviceChangedHandler(onOffEntry, nullptr, &onOffCmd);

    // 2. Identify Cluster
    Clusters::Binding::TableEntry identifyEntry;
    identifyEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    identifyEntry.local     = 1;
    identifyEntry.remote    = 2;
    identifyEntry.clusterId = Clusters::Identify::Id;

    Clusters::Identify::Commands::Identify::Type identifyCmd;
    identifyCmd.identifyTime = 5;
    SimpleBindingCommand<Clusters::Identify::Commands::Identify::Type> identifyBindingCmd(identifyCmd);
    EXPECT_EQ(identifyBindingCmd.GetClusterId(), Clusters::Identify::Id);
    AllDevicesBoundDeviceChangedHandler(identifyEntry, nullptr, &identifyBindingCmd);

    // 3. Chime Cluster
    Clusters::Binding::TableEntry chimeEntry;
    chimeEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    chimeEntry.local     = 1;
    chimeEntry.remote    = 2;
    chimeEntry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> chimeBindingCmd(chimeCmd);
    EXPECT_EQ(chimeBindingCmd.GetClusterId(), Clusters::Chime::Id);
    AllDevicesBoundDeviceChangedHandler(chimeEntry, nullptr, &chimeBindingCmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesClusterMismatch)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::OnOff::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    // Passing a Chime command to an OnOff binding entry should detect mismatch and not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesWildcardBinding)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = std::nullopt; // Wildcard binding

    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> cmd(toggleCmd);

    // Wildcard binding should adopt the command's cluster ID and execute without crashing
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesUnsupportedBindingType)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNUSED_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, DispatchAndScheduleBindingCommandNullValidation)
{
    EXPECT_EQ(DispatchBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ScheduleBindingCommand(DeviceLayer::PlatformMgr(), Clusters::Binding::Manager::GetInstance(), 1, nullptr),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestBindingHandler, TriggerChimeUnregisteredFails)
{
    class DummyIdentifyDelegate : public Clusters::IdentifyDelegate
    {
    public:
        void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override {}
        void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override {}
        void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override {}
        bool IsTriggerEffectEnabled() const override { return false; }
    } dummyIdentify;

    DefaultTimerDelegate timerDelegate;
    Doorbell::Config config{
        .timerDelegate    = timerDelegate,
        .platformManager  = DeviceLayer::PlatformMgr(),
        .bindingTable     = Clusters::Binding::Table::GetInstance(),
        .bindingManager   = Clusters::Binding::Manager::GetInstance(),
        .identifyDelegate = dummyIdentify,
    };
    Doorbell doorbell(config);

    // Prior to registration, TriggerChime must return CHIP_ERROR_INCORRECT_STATE
    EXPECT_EQ(doorbell.TriggerChime(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_ERROR_INCORRECT_STATE);
}

} // namespace

