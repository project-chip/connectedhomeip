/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <algorithm>
#include <functional>
#include <stdint.h>
#include <utility>

#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataList.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>

using namespace chip;

namespace {

class TestMetadataList : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

template <typename T>
struct IdAndValue
{
    uint32_t id;
    T value;

    bool operator==(const IdAndValue & other) const
    {
        return (this == &other) || ((this->id == other.id) && (this->value == other.value));
    }
};

TEST_F(TestMetadataList, MetadataListWorks)
{
    MetadataList<IdAndValue<int>> list1;
    EXPECT_EQ(list1.size(), 0u);
    EXPECT_TRUE(list1.empty());

    // Reservation should always work when empty
    ASSERT_EQ(list1.reserve(5), CHIP_NO_ERROR);
    EXPECT_EQ(list1.capacity(), 5u);
    EXPECT_EQ(list1.size(), 0u);
    EXPECT_TRUE(list1.empty());

    // You can re-reserve differently if still empty.
    ASSERT_EQ(list1.reserve(2), CHIP_NO_ERROR);
    EXPECT_EQ(list1.capacity(), 2u);

    // Values can be appended until the capacity.
    EXPECT_EQ(list1.Append({ 0xA1, 111 }), CHIP_NO_ERROR);
    EXPECT_EQ(list1.size(), 1u);

    EXPECT_EQ(list1.Append({ 0xA2, 222 }), CHIP_NO_ERROR);
    EXPECT_EQ(list1.size(), 2u);

    EXPECT_EQ(list1.Append({ 0xA3, 333 }), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(list1.size(), 2u);

    MetadataList<IdAndValue<int>> list2 = std::move(list1);

    // Moved-from list is "empty", un-Metadata and span is empty.
    EXPECT_EQ(list1.size(), 0u);
    EXPECT_EQ(list1.capacity(), 0u);
    EXPECT_TRUE(list1.GetSpanValidForLifetime().empty());

    // Moved-to list has storage.
    EXPECT_EQ(list2.size(), 2u);

    // A span can be obtained over the list.
    decltype(list2)::SpanType contents = list2.GetSpanValidForLifetime();
    EXPECT_EQ(contents.size(), 2u);

    size_t idx = 0;
    for (const auto & element : contents)
    {
        size_t oneBasedIndex = idx + 1;
        EXPECT_EQ(element.id, 0xA0u + oneBasedIndex);
        EXPECT_EQ(element.value, 111 * static_cast<int>(oneBasedIndex));
        ++idx;
    }
    EXPECT_EQ(idx, 2u);

    // After getting a span, list becomes immutable and it is no longer possible to append to the list.
    EXPECT_EQ(list2.Append({ 0xA3, 333 }), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(list2.size(), 2u);
    EXPECT_EQ(list2.capacity(), 2u);

    // Cannot re-reserve once the list has become immutable due to span-taking.
    EXPECT_EQ(list2.reserve(6), CHIP_ERROR_INCORRECT_STATE);
}

static constexpr std::array<const int, 3> kConstantArray{ 1, 2, 3 };

TEST_F(TestMetadataList, MetadataListConvertersWork)
{
    {
        MetadataList<int> list{ MetadataList<int>::FromArray(std::array{ 1, 2, 3 }) };
        EXPECT_FALSE(list.empty());
        EXPECT_EQ(list.size(), 3u);
        EXPECT_EQ(list[0], 1);
        EXPECT_EQ(list[1], 2);
        EXPECT_EQ(list[2], 3);

        auto list2 = std::move(list);
        EXPECT_EQ(list.size(), 0u);
        auto list2Span = list2.GetSpanValidForLifetime();
        EXPECT_EQ(list2Span.size(), 3u);
        EXPECT_EQ(list2Span[0], 1);
        EXPECT_EQ(list2Span[1], 2);
        EXPECT_EQ(list2Span[2], 3);

        EXPECT_EQ(list2.reserve(10), CHIP_ERROR_INCORRECT_STATE);
        EXPECT_EQ(list2.Append(4), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        MetadataList<int> list1{ MetadataList<int>::FromConstArray(kConstantArray) };
        EXPECT_EQ(list1.size(), 3u);
        EXPECT_EQ(list1[0], 1);
        EXPECT_EQ(list1[1], 2);
        EXPECT_EQ(list1[2], 3);

        EXPECT_EQ(list1.reserve(10), CHIP_ERROR_INCORRECT_STATE);
        EXPECT_EQ(list1.Append(4), CHIP_ERROR_INCORRECT_STATE);

        MetadataList<int> list2{ MetadataList<int>::FromConstArray(kConstantArray) };
        EXPECT_EQ(list2.size(), 3u);
        EXPECT_EQ(list2[0], 1);
        EXPECT_EQ(list2[1], 2);
        EXPECT_EQ(list2[2], 3);

        auto list1Span = list1.GetSpanValidForLifetime();
        auto list2Span = list2.GetSpanValidForLifetime();

        EXPECT_EQ(list1Span.data(), list2Span.data());
        EXPECT_EQ(list1Span.size(), list2Span.size());
        EXPECT_EQ(list1Span.data(), kConstantArray.data());
    }

    {
        MetadataList<int> list1{ MetadataList<int>::FromConstSpan(Span<const int>{ kConstantArray }) };
        EXPECT_EQ(list1.size(), 3u);
        EXPECT_EQ(list1[0], 1);
        EXPECT_EQ(list1[1], 2);
        EXPECT_EQ(list1[2], 3);

        EXPECT_EQ(list1.reserve(10), CHIP_ERROR_INCORRECT_STATE);
        EXPECT_EQ(list1.Append(4), CHIP_ERROR_INCORRECT_STATE);

        MetadataList<int> list2{ MetadataList<int>::FromConstSpan(Span<const int>{ kConstantArray }) };
        EXPECT_EQ(list2.size(), 3u);
        EXPECT_EQ(list2[0], 1);
        EXPECT_EQ(list2[1], 2);
        EXPECT_EQ(list2[2], 3);

        auto list1Span = list1.GetSpanValidForLifetime();
        auto list2Span = list2.GetSpanValidForLifetime();

        EXPECT_EQ(list1Span.data(), list2Span.data());
        EXPECT_EQ(list1Span.size(), list2Span.size());
        EXPECT_EQ(list1Span.data(), kConstantArray.data());
    }
}

enum MinCommandPrivilege : uint8_t
{
    kOperate = 0u,
    kManage  = 1u,
    kAdmin   = 2u,

    kMax = kAdmin
};

static constexpr uint16_t kPrivilegeFieldWidth = 0x3;
static constexpr uint16_t kPrivilegeFieldMask  = static_cast<uint16_t>((1u << kPrivilegeFieldWidth) - 1);
static_assert(MinCommandPrivilege::kMax <= kPrivilegeFieldMask, "Privilege mask is not wide enough");

// Bitmask values for different Command qualities.
enum class CommandMetadataFlags : uint16_t
{
    kFabricScoped = 0x1 << 0,
    kTimed        = 0x1 << 1, // `T` quality on commands
    kLargeMessage = 0x1 << 2, // `L` quality on commands
    kIsResponse   = 0x1 << 3, // Command is server => client response

    kMinPrivilegeValueMask = kPrivilegeFieldMask << 4

};

typedef BitMask<CommandMetadataFlags> CommandMetadata;

// MinCommandPrivilege GetMinCommandPrivilege(CommandMetadata metadata) const {
//   return static_cast<MinCommandPrivilege>(metadata.GetField(CommandMetadataFlags::kMinPrivilegeValueMask));
// }

struct CommandEntry
{
    CommandId mei;
    CommandMetadata metadata;
};

enum class NetworkCommissioningFeatureBits : uint8_t
{
    // TODO: NOT REAL VALUEs
    kWifi     = 1 << 0,
    kThread   = 1 << 1,
    kEthernet = 1 << 2,
};

enum NetworkCommissioningCommands : CommandId
{
    kScanNetworks             = 0x00u, // | client => server | ScanNetworksResponse   | A      | WI \| TH
    kScanNetworksResponse     = 0x01u, // | client <= server | N                      |        | WI \| TH
    kAddOrUpdateWiFiNetwork   = 0x02u, // | client => server | NetworkConfigResponse  | A      | WI
    kAddOrUpdateThreadNetwork = 0x03u, // | client => server | NetworkConfigResponse  | A      | TH
    kRemoveNetwork            = 0x04u, // | client => server | NetworkConfigResponse  | A      | WI \| TH
    kNetworkConfigResponse    = 0x05u, // | client <= server | N                      |        | WI \| TH
    kConnectNetwork           = 0x06u, // | client => server | ConnectNetworkResponse | A      | WI \| TH
    kConnectNetworkResponse   = 0x07u, // | client <= server | N                      |        | WI \| TH
    kReorderNetwork           = 0x08u, // | client => server | NetworkConfigResponse  | A      | WI \| TH

};

static const std::array<CommandEntry, 9> kAllCommands{
    CommandEntry{ kScanNetworks,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
    CommandEntry{ kScanNetworksResponse, CommandMetadata{}.Set(CommandMetadataFlags::kIsResponse) },
    CommandEntry{ kAddOrUpdateWiFiNetwork,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
    CommandEntry{ kAddOrUpdateThreadNetwork,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
    CommandEntry{ kRemoveNetwork,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
    CommandEntry{ kNetworkConfigResponse, CommandMetadata{}.Set(CommandMetadataFlags::kIsResponse) },
    CommandEntry{ kConnectNetwork,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
    CommandEntry{ kConnectNetworkResponse, CommandMetadata{}.Set(CommandMetadataFlags::kIsResponse) },
    CommandEntry{ kReorderNetwork,
                  CommandMetadata{}.SetField(CommandMetadataFlags::kMinPrivilegeValueMask, MinCommandPrivilege::kAdmin) },
};

using FilterPredicate = bool (*)(void * context, void * object);
template <typename T>
MetadataList<T> FilterElements(Span<const T> elementTable, std::function<bool(const T &)> supportedPredicate)
{
    MetadataList<T> result;

    if (result.reserve(elementTable.size()) != CHIP_NO_ERROR)
    {
        result.Invalidate();
        return result;
    }

    for (const auto & element : elementTable)
    {
        if (!supportedPredicate(element))
        {
            continue;
        }

        // Append as much as we can
        (void) result.Append(element);
    }

    return result;
}

class MiniCluster
{
public:
    MiniCluster() {}
    virtual ~MiniCluster() {}

    virtual MetadataList<CommandEntry> GetSupportedCommands() const
    {
        return FilterElements<CommandEntry>(Span<const CommandEntry>{ kAllCommands.data(), kAllCommands.size() },
                                            [this](const CommandEntry & entry) { return this->IsCommandSupported(entry.mei); });
    }

#if 0
    virtual bool IsCommandSupported(CommandId commandId) const
    {
        using Commands = NetworkCommissioningCommands;

        if (commandId > Commands::kReorderNetwork)
        {
            return false;
        }

        if (mFeatures.HasAny(NetworkCommissioningFeatureBits::kWifi, NetworkCommissioningFeatureBits::kThread))
        {
            if ((commandId >= Commands::kScanNetworks) && (commandId <= Commands::kScanNetworksResponse))
            {
                return true;
            }

            if ((commandId >= Commands::kRemoveNetwork) && (commandId <= Commands::kReorderNetwork))
            {
                return true;
            }
        }

        if (mFeatures.Has(NetworkCommissioningFeatureBits::kWifi))
        {
            if (commandId == Commands::kAddOrUpdateWiFiNetwork)
            {
                return true;
            }
        }

        if (mFeatures.Has(NetworkCommissioningFeatureBits::kThread))
        {
            if (commandId == Commands::kAddOrUpdateThreadNetwork)
            {
                return true;
            }
        }

        return false;
    }
#endif // 0

    virtual bool IsCommandSupported(CommandId commandId) const
    {
        using Commands = NetworkCommissioningCommands;

        switch (commandId)
        {
        case Commands::kScanNetworks:
        case Commands::kScanNetworksResponse:
        case Commands::kRemoveNetwork:
        case Commands::kNetworkConfigResponse:
        case Commands::kConnectNetwork:
        case Commands::kConnectNetworkResponse:
        case Commands::kReorderNetwork:
            return mFeatures.HasAny(NetworkCommissioningFeatureBits::kWifi, NetworkCommissioningFeatureBits::kThread);
        case Commands::kAddOrUpdateWiFiNetwork:
            return mFeatures.Has(NetworkCommissioningFeatureBits::kWifi);
        case Commands::kAddOrUpdateThreadNetwork:
            return mFeatures.Has(NetworkCommissioningFeatureBits::kThread);
        default:
            return false;
        }
    }

    void SetFeatures(BitFlags<NetworkCommissioningFeatureBits> features) { mFeatures = features; }

private:
    BitFlags<NetworkCommissioningFeatureBits> mFeatures{};
};

TEST_F(TestMetadataList, CommandsForFeaturesAreAsExpected)
{
    MiniCluster cluster;

    {
        cluster.SetFeatures(BitFlags<NetworkCommissioningFeatureBits>{}.Set(NetworkCommissioningFeatureBits::kWifi));

        auto supportedCommands = cluster.GetSupportedCommands();
        EXPECT_EQ(supportedCommands.size(), 8u);
    }

    {
        cluster.SetFeatures(BitFlags<NetworkCommissioningFeatureBits>{}.Set(NetworkCommissioningFeatureBits::kEthernet));

        auto supportedCommands = cluster.GetSupportedCommands();
        EXPECT_EQ(supportedCommands.size(), 0u);
    }
}

} // namespace
