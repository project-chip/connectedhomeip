/*
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
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/AttributeChangeListener.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

using chip::Protocols::InteractionModel::Status;

// Minimal Concrete Provider for testing
class TestProvider : public Provider
{
public:
    // Implement pure virtuals from ProviderMetadataTree
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<EndpointEntry> & builder) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DeviceTypeEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & builder) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ServerClusterEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, EventEntry & eventInfo) override { return CHIP_NO_ERROR; }
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AttributeEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AcceptedCommandEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    // Implement pure virtuals from Provider
    ActionReturnStatus ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder) override
    {
        return Status::Success;
    }
    ActionReturnStatus WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder) override
    {
        return Status::Success;
    }
    void ListAttributeWriteNotification(const ConcreteAttributePath & aPath, ListWriteOperation opType,
                                        FabricIndex accessingFabric) override
    {}
    std::optional<ActionReturnStatus> InvokeCommand(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                                                    CommandHandler * handler) override
    {
        return Status::Success;
    }
};

// Mock Listener
class TestListener : public AttributeChangeListener
{
public:
    int callCount = 0;
    ConcreteAttributePath lastPath;
    AttributeChangeType lastType;

    void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
    {
        callCount++;
        lastPath = path;
        lastType = type;
    }
};

TEST(TestProviderListener, TestSingleListener)
{
    TestProvider provider;
    TestListener listener;

    provider.RegisterAttributeChangeListener(listener);

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listener.callCount, 1);
    EXPECT_EQ(listener.lastPath.mEndpointId, 1u);
    EXPECT_EQ(listener.lastPath.mClusterId, 1u);
    EXPECT_EQ(listener.lastPath.mAttributeId, 1u);
    EXPECT_EQ(listener.lastType, AttributeChangeType::kReportable);

    provider.UnregisterAttributeChangeListener(listener);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kQuiet);
    EXPECT_EQ(listener.callCount, 1); // Should not have incremented
}

TEST(TestProviderListener, TestMultipleListeners)
{
    TestProvider provider;
    TestListener listener1;
    TestListener listener2;

    provider.RegisterAttributeChangeListener(listener1);
    provider.RegisterAttributeChangeListener(listener2);

    ConcreteAttributePath path(1, 2, 3);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kQuiet);

    EXPECT_EQ(listener1.callCount, 1);
    EXPECT_EQ(listener1.lastPath.mEndpointId, 1u);
    EXPECT_EQ(listener1.lastType, AttributeChangeType::kQuiet);

    EXPECT_EQ(listener2.callCount, 1);
    EXPECT_EQ(listener2.lastPath.mEndpointId, 1u);
    EXPECT_EQ(listener2.lastType, AttributeChangeType::kQuiet);

    provider.UnregisterAttributeChangeListener(listener1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listener1.callCount, 1); // Unregistered
    EXPECT_EQ(listener2.callCount, 2); // Still registered
    EXPECT_EQ(listener2.lastType, AttributeChangeType::kReportable);
}

TEST(TestProviderListener, TestUnregisterNextListener)
{
    TestProvider provider;
    TestListener listener1;

    class UnregisteringListener : public TestListener
    {
    public:
        TestProvider * provider;
        AttributeChangeListener * target;
        void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
        {
            TestListener::OnAttributeChanged(path, type);
            if (target)
            {
                provider->UnregisterAttributeChangeListener(*target);
            }
        }
    };

    UnregisteringListener listenerHead;
    listenerHead.provider = &provider;
    listenerHead.target   = &listener1;

    provider.RegisterAttributeChangeListener(listener1);
    provider.RegisterAttributeChangeListener(listenerHead);

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listenerHead.callCount, 1);
    EXPECT_EQ(listener1.callCount, 0);
}

TEST(TestProviderListener, TestUnregisterSelf)
{
    TestProvider provider;

    class SelfUnregisteringListener : public TestListener
    {
    public:
        TestProvider * provider;
        void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
        {
            TestListener::OnAttributeChanged(path, type);
            provider->UnregisterAttributeChangeListener(*this);
        }
    };

    SelfUnregisteringListener listener;
    listener.provider = &provider;

    provider.RegisterAttributeChangeListener(listener);

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listener.callCount, 1);

    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);
    EXPECT_EQ(listener.callCount, 1);
}

TEST(TestProviderListener, TestUnregisterPreviousListener)
{
    TestProvider provider;
    TestListener listener1;

    class UnregisteringListener : public TestListener
    {
    public:
        TestProvider * provider;
        AttributeChangeListener * target;
        void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
        {
            TestListener::OnAttributeChanged(path, type);
            if (target)
            {
                provider->UnregisterAttributeChangeListener(*target);
            }
        }
    };

    UnregisteringListener listenerTail;
    listenerTail.provider = &provider;
    listenerTail.target   = &listener1;

    provider.RegisterAttributeChangeListener(listenerTail);
    provider.RegisterAttributeChangeListener(listener1);

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listener1.callCount, 1);
    EXPECT_EQ(listenerTail.callCount, 1);

    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listener1.callCount, 1);
    EXPECT_EQ(listenerTail.callCount, 2);
}

TEST(TestProviderListener, TestNestedNotifications)
{
    TestProvider provider;
    TestListener listener1;

    class NestedListener : public TestListener
    {
    public:
        TestProvider * provider;
        void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
        {
            TestListener::OnAttributeChanged(path, type);
            if (callCount == 1)
            {
                provider->NotifyAttributeChanged(path, type);
            }
        }
    };

    NestedListener listenerHead;
    listenerHead.provider = &provider;

    provider.RegisterAttributeChangeListener(listener1);
    provider.RegisterAttributeChangeListener(listenerHead);

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(listenerHead.callCount, 2);
    EXPECT_EQ(listener1.callCount, 2);
}

TEST(TestProviderListener, TestStressNestingAndRemoval)
{
    TestProvider provider;

    struct StressListener : public TestListener
    {
        int id;
        TestProvider * provider;
        StressListener * targetToRemove = nullptr;
        StressListener * targetToAdd    = nullptr;
        bool triggerNested              = false;
        int * callOrderArray;
        int * callOrderIndex;

        StressListener(int i, TestProvider * p, int * arr, int * idx) : id(i), provider(p), callOrderArray(arr), callOrderIndex(idx)
        {}

        void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
        {
            TestListener::OnAttributeChanged(path, type);
            callOrderArray[(*callOrderIndex)++] = id;

            if (triggerNested)
            {
                triggerNested = false; // avoid infinite loop
                provider->NotifyAttributeChanged(path, type);
            }

            if (targetToRemove)
            {
                provider->UnregisterAttributeChangeListener(*targetToRemove);
            }

            if (targetToAdd)
            {
                provider->RegisterAttributeChangeListener(*targetToAdd);
            }
        }
    };

    int callOrder[20] = { 0 };
    int callIndex     = 0;

    StressListener l1(1, &provider, callOrder, &callIndex);
    StressListener l2(2, &provider, callOrder, &callIndex);
    StressListener l3(3, &provider, callOrder, &callIndex);
    StressListener l4(4, &provider, callOrder, &callIndex);
    StressListener l5(5, &provider, callOrder, &callIndex);

    provider.RegisterAttributeChangeListener(l1);
    provider.RegisterAttributeChangeListener(l2);
    provider.RegisterAttributeChangeListener(l3);
    provider.RegisterAttributeChangeListener(l4);
    provider.RegisterAttributeChangeListener(l5);

    // Scenario setup:
    // Listeners are registered in order L1, L2, L3, L4, L5.
    // Since Register adds to the head, the list order is L5 -> L4 -> L3 -> L2 -> L1.
    //
    // 1. Outer notification starts. L5 is processed first.
    // 2. L5 triggers a nested notification.
    // 3. In the nested notification:
    //    - L5 is processed again (triggerNested reset to false).
    //    - L4 is processed. It removes L3.
    //      - This updates the nested iterator's expectedNext from L3 to L2.
    //      - This also updates L4's next pointer to L2.
    //    - L3 is skipped in the nested notification because it was removed.
    //    - L2 is processed. It removes itself.
    //      - This updates L4's next pointer to L1.
    //    - L1 is processed. It removes L5 (the head/outer current).
    // 4. Nested notification ends.
    // 5. Outer notification resumes.
    //    - The outer iterator was pointing to L4 as the next element before L5 called nested.
    //    - So L4 is processed in the outer loop.
    //    - L4 tries to remove L3 again (no-op).
    //    - Next element for outer iterator is L4->next, which is now L1 (since L3 and L2 were removed).
    //    - So L3 and L2 are skipped in the outer loop.
    //    - L1 is processed in the outer loop.
    //    - L1 tries to remove L5 again (no-op).
    // 6. Outer notification ends.
    //
    // Expected call order:
    // Outer L5 -> Nested L5 -> Nested L4 -> Nested L2 -> Nested L1 -> Outer L4 -> Outer L1

    l5.triggerNested  = true;
    l4.targetToRemove = &l3;
    l2.targetToRemove = &l2;
    l1.targetToRemove = &l5;

    ConcreteAttributePath path(1, 1, 1);
    provider.NotifyAttributeChanged(path, AttributeChangeType::kReportable);

    EXPECT_EQ(callIndex, 7);

    EXPECT_EQ(callOrder[0], 5); // Outer L5
    EXPECT_EQ(callOrder[1], 5); // Nested L5
    EXPECT_EQ(callOrder[2], 4); // Nested L4
    EXPECT_EQ(callOrder[3], 2); // Nested L2
    EXPECT_EQ(callOrder[4], 1); // Nested L1
    EXPECT_EQ(callOrder[5], 4); // Outer L4
    EXPECT_EQ(callOrder[6], 1); // Outer L1
}

} // namespace
