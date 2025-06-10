#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/endpoint/EndpointProviderRegistry.h>
#include <data-model-providers/endpoint/SpanEndpointProvider.h>

#include <algorithm>
#include <array>
#include <list>
#include <random>
#include <vector>

using namespace chip;
using namespace chip::app;

// Mock ServerClusterInterface for testing, ensuring Descriptor cluster presence
namespace {

class MinimalServerCluster : public DefaultServerCluster
{
public:
    MinimalServerCluster(EndpointId endpointId, ClusterId clusterId) :
        DefaultServerCluster(ConcreteClusterPath(endpointId, clusterId)), mPath(endpointId, clusterId)
    {}
    ~MinimalServerCluster() override = default;

    chip::Span<const ConcreteClusterPath> GetPaths() const override { return chip::Span<const ConcreteClusterPath>(&mPath, 1); }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        return DataModel::ActionReturnStatus(CHIP_ERROR_NOT_IMPLEMENTED);
    }

private:
    ConcreteClusterPath mPath;
};

constexpr EndpointId kTestEndpointId1    = 1;
constexpr EndpointId kTestEndpointId2    = 2;
constexpr EndpointId kNonExistentId      = 3;
constexpr EndpointId kValidIdForArgsTest = 1; // Same as kTestEndpointId1, but for clarity in RegisterInvalidArgs
constexpr EndpointId kListId1ForArgsTest = 10;
constexpr EndpointId kListId2ForArgsTest = 11;

MinimalServerCluster descriptorCluster(kTestEndpointId1, chip::app::Clusters::Descriptor::Id);
std::array<ServerClusterInterface *, 1> serverClustersArrayWithDescriptor = { &descriptorCluster };

} // namespace

TEST(TestEndpointProviderRegistry, CreateAndDestroy)
{
    EndpointProviderRegistry registry;

    // Create a provider
    auto build_result = SpanEndpointProvider::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result));
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result)));
    ASSERT_NE(provider, nullptr);

    EndpointProviderInterface * provider_ptr = provider.get(); // Save raw pointer for comparison

    // Register it
    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Check if it can be retrieved
    ASSERT_EQ(registry.Get(kTestEndpointId1), provider_ptr);

    // Unregister the provider
    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR);

    // Destroy the provider
    provider.reset();

    // Check if it is no longer retrievable
    // After unregistering, Get(kTestEndpointId1) should return nullptr.
    ASSERT_EQ(registry.Get(kTestEndpointId1), nullptr);
}

TEST(TestEndpointProviderRegistry, RegisterMultipleProviders)
{
    EndpointProviderRegistry registry;

    auto build_result1 = SpanEndpointProvider::Builder(kTestEndpointId1)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                             .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result1));
    auto provider1     = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result1)));
    auto build_result2 = SpanEndpointProvider::Builder(kTestEndpointId2)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                             .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result2));
    auto provider2 = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result2)));

    ASSERT_NE(provider1, nullptr);
    ASSERT_NE(provider2, nullptr);

    EndpointProviderRegistration registration1(*provider1);
    EndpointProviderRegistration registration2(*provider2);

    ASSERT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Register(registration2), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Get(kTestEndpointId1), provider1.get());
    ASSERT_EQ(registry.Get(kTestEndpointId2), provider2.get());

    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId1), nullptr);
    ASSERT_EQ(registry.Get(kTestEndpointId2), provider2.get());

    ASSERT_EQ(registry.Unregister(kTestEndpointId2), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId2), nullptr);
}

TEST(TestEndpointProviderRegistry, RegisterDuplicateProviderId)
{
    EndpointProviderRegistry registry;

    auto build_result1a = SpanEndpointProvider::Builder(kTestEndpointId1)
                              .SetServerClusters(serverClustersArrayWithDescriptor)
                              .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                              .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result1a));
    auto provider1a = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result1a)));

    auto build_result1b = SpanEndpointProvider::Builder(kTestEndpointId1)
                              .SetServerClusters(serverClustersArrayWithDescriptor)
                              .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                              .build(); // Same ID
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result1b));
    auto provider1b = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result1b)));

    ASSERT_NE(provider1a, nullptr);
    ASSERT_NE(provider1b, nullptr);

    EndpointProviderRegistration registration1a(*provider1a);
    EndpointProviderRegistration registration1b(*provider1b);

    ASSERT_EQ(registry.Register(registration1a), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId1), provider1a.get());

    // Attempt to register another provider with the same ID
    ASSERT_EQ(registry.Register(registration1b), CHIP_ERROR_DUPLICATE_KEY_ID);
    ASSERT_EQ(registry.Get(kTestEndpointId1), provider1a.get()); // Should still be the first one
}

TEST(TestEndpointProviderRegistry, RegisterSameRegistrationObject)
{
    EndpointProviderRegistry registry;
    auto build_result = SpanEndpointProvider::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result));
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result)));
    ASSERT_NE(provider, nullptr);

    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Attempt to register the exact same registration object again
    ASSERT_EQ(registry.Register(registration), CHIP_ERROR_DUPLICATE_KEY_ID); // Because registration.next is no longer nullptr
}

TEST(TestEndpointProviderRegistry, UnregisterNonExistentProvider)
{
    EndpointProviderRegistry registry;
    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR); // Should succeed, no error for non-existent ID

    auto build_result = SpanEndpointProvider::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result));
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result)));
    ASSERT_NE(provider, nullptr);
    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Unregister(kTestEndpointId2), CHIP_NO_ERROR); // Different ID. Should still succeed
}

TEST(TestEndpointProviderRegistry, GetNonExistentProvider)
{
    EndpointProviderRegistry registry;
    ASSERT_EQ(registry.Get(kNonExistentId), nullptr);
}

TEST(TestEndpointProviderRegistry, IteratorTest)
{
    EndpointProviderRegistry registry;

    auto build_result1 = SpanEndpointProvider::Builder(kTestEndpointId1)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                             .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result1));
    auto provider1 = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result1)));

    auto build_result2 = SpanEndpointProvider::Builder(2)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                             .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result2));
    auto provider2 = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result2)));

    EndpointProviderRegistration registration1(*provider1);
    EndpointProviderRegistration registration2(*provider2);

    registry.Register(registration1);
    registry.Register(registration2); // Registry stores in LIFO order for simple list prepend

    size_t count = 0;
    bool found1  = false;
    bool found2  = false;
    for (auto * ep : registry)
    {
        ASSERT_NE(ep, nullptr);
        if (ep->GetEndpointEntry().id == kTestEndpointId1)
            found1 = true;
        if (ep->GetEndpointEntry().id == kTestEndpointId2)
            found2 = true;
        count++;
    }
    ASSERT_EQ(count, 2u);
    ASSERT_TRUE(found1);
    ASSERT_TRUE(found2);

    // Test empty iteration
    EndpointProviderRegistry emptyRegistry;
    ASSERT_EQ(emptyRegistry.begin(), emptyRegistry.end());
}

TEST(TestEndpointProviderRegistry, RegisterInvalidArgs)
{
    EndpointProviderRegistry registry;
    auto build_result_valid = SpanEndpointProvider::Builder(kValidIdForArgsTest)
                                  .SetServerClusters(serverClustersArrayWithDescriptor)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result_valid));
    auto providerValid = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result_valid)));
    ASSERT_NE(providerValid, nullptr);

    // Case 2: EndpointProviderInterface returns kInvalidEndpointId
    auto build_result_invalid = SpanEndpointProvider::Builder(kInvalidEndpointId).build();
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(build_result_invalid)); // Expecting an error for invalid ID
    ASSERT_EQ(std::get<CHIP_ERROR>(build_result_invalid), CHIP_ERROR_INVALID_ARGUMENT);
    // Cannot create a provider from an error, so this part of the test needs adjustment.
    // We'll test that registering a provider that *would* have an invalid ID (if it could be built)
    // is caught by the registry if we somehow bypassed the builder's check.
    // For now, the builder prevents this. If we want to test the registry's check directly,
    // we'd need a mock provider that returns kInvalidEndpointId.
    // The current SpanEndpointProvider::Builder already prevents creation with kInvalidEndpointId.

    // Case 3: EndpointProviderRegistration already part of a list (entry.next != nullptr)
    // To test this, we need two valid providers and registrations.
    auto build_result_list1 = SpanEndpointProvider::Builder(kListId1ForArgsTest)
                                  .SetServerClusters(serverClustersArrayWithDescriptor)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result_list1));
    auto providerForList1   = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result_list1)));
    auto build_result_list2 = SpanEndpointProvider::Builder(kListId2ForArgsTest)
                                  .SetServerClusters(serverClustersArrayWithDescriptor)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result_list2));
    auto providerForList2 = std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result_list2)));
    EndpointProviderRegistration registrationInList1(*providerForList1);
    EndpointProviderRegistration registrationInList2(*providerForList2);

    // Manually link to simulate it being in another list
    // This simulates registrationInList2 being part of a list where registrationInList1 is its successor.
    registrationInList2.next = &registrationInList1;
    EXPECT_EQ(registry.Register(registrationInList2), CHIP_ERROR_INVALID_ARGUMENT);
    registrationInList2.next = nullptr; // Reset for other tests or cleanup

    // Test that registering a valid provider after these failures still works
    EndpointProviderRegistration registrationValid(*providerValid);
    EXPECT_EQ(registry.Register(registrationValid), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Get(kValidIdForArgsTest), providerValid.get());
}

TEST(TestEndpointProviderRegistry, StressTestRegistration)
{
    constexpr int kNumProviders = 100;
    EndpointProviderRegistry registry;
    std::vector<std::unique_ptr<SpanEndpointProvider>> providers_storage; // Owns the providers
    std::list<EndpointProviderRegistration> registrations;                // Owns the registration objects, stable addresses
    std::vector<EndpointId> ids;

    providers_storage.reserve(kNumProviders);
    for (int i = 0; i < kNumProviders; ++i)
    {
        EndpointId id     = static_cast<EndpointId>(i + 1);
        auto build_result = SpanEndpointProvider::Builder(id)
                                .SetServerClusters(serverClustersArrayWithDescriptor)
                                .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                .build();
        ASSERT_TRUE(std::holds_alternative<SpanEndpointProvider>(build_result));
        providers_storage.push_back(
            std::make_unique<SpanEndpointProvider>(std::move(std::get<SpanEndpointProvider>(build_result))));
        ids.push_back(id);
        registrations.emplace_back(*providers_storage.back()); // Create registration from the provider
    }

    // Register all
    auto reg_it = registrations.begin();
    for (size_t i = 0; i < static_cast<size_t>(kNumProviders); ++i, ++reg_it)
    {
        ASSERT_EQ(registry.Register(*reg_it), CHIP_NO_ERROR) << "Failed to register provider with ID " << ids[i];
    }

    // Verify all can be retrieved
    std::vector<EndpointId> shuffled_ids = ids;
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), std::default_random_engine(0));
    for (EndpointId id : shuffled_ids)
    {
        ASSERT_NE(registry.Get(id), nullptr) << "Failed to get provider with ID " << id;
        ASSERT_EQ(registry.Get(id)->GetEndpointEntry().id, id);
    }

    // Unregister all in shuffled order
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), std::default_random_engine(1));
    for (EndpointId id : shuffled_ids)
    {
        ASSERT_EQ(registry.Unregister(id), CHIP_NO_ERROR) << "Failed to unregister provider with ID " << id;
        ASSERT_EQ(registry.Get(id), nullptr) << "Provider with ID " << id << " still found after unregistration";
    }

    // Re-register all
    // Reset next pointers in registrations before re-registering
    for (auto & reg : registrations)
    {
        reg.next = nullptr;
    }
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), std::default_random_engine(2));
    for (EndpointId id_to_register : shuffled_ids)
    {
        bool found_and_registered = false;
        for (auto & reg : registrations)
        { // Iterate through the list of registrations
            if (reg.endpointProviderInterface->GetEndpointEntry().id == id_to_register)
            {
                ASSERT_EQ(registry.Register(reg), CHIP_NO_ERROR) << "Failed to re-register provider with ID " << id_to_register;
                found_and_registered = true;
                break;
            }
        }
        ASSERT_TRUE(found_and_registered);
    }

    for (EndpointId id : ids)
    {
        ASSERT_NE(registry.Get(id), nullptr) << "Failed to get provider with ID " << id << " after re-registration";
    }
}
