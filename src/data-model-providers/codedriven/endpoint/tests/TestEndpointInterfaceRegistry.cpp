#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>

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

TEST(TestEndpointInterfaceRegistry, CreateAndDestroy)
{
    EndpointInterfaceRegistry registry;

    // Create a provider
    auto build_result = SpanEndpoint::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result));
    auto provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result)));
    ASSERT_NE(provider, nullptr);

    EndpointInterface * provider_ptr = provider.get(); // Save raw pointer for comparison

    // Register it
    EndpointInterfaceRegistration registration(*provider);
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

TEST(TestEndpointInterfaceRegistry, RegisterMultipleProviders)
{
    EndpointInterfaceRegistry registry;

    auto build_result1 = SpanEndpoint::Builder(kTestEndpointId1)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                             .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result1));
    auto provider1 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result1)));

    MinimalServerCluster descriptorCluster2(kTestEndpointId2, chip::app::Clusters::Descriptor::Id);
    std::array<ServerClusterInterface *, 1> serverClustersArrayWithDescriptor2 = { &descriptorCluster2 };

    auto build_result2 = SpanEndpoint::Builder(kTestEndpointId2)
                             .SetServerClusters(serverClustersArrayWithDescriptor2)
                             .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                             .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result2));
    auto provider2 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result2)));

    ASSERT_NE(provider1, nullptr);
    ASSERT_NE(provider2, nullptr);

    EndpointInterfaceRegistration registration1(*provider1);
    EndpointInterfaceRegistration registration2(*provider2);

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

TEST(TestEndpointInterfaceRegistry, RegisterDuplicateProviderId)
{
    EndpointInterfaceRegistry registry;

    auto build_result1a = SpanEndpoint::Builder(kTestEndpointId1)
                              .SetServerClusters(serverClustersArrayWithDescriptor)
                              .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                              .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result1a));
    auto provider1a = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result1a)));

    auto build_result1b = SpanEndpoint::Builder(kTestEndpointId1)
                              .SetServerClusters(serverClustersArrayWithDescriptor)
                              .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                              .Build(); // Same ID
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result1b));
    auto provider1b = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result1b)));

    ASSERT_NE(provider1a, nullptr);
    ASSERT_NE(provider1b, nullptr);

    EndpointInterfaceRegistration registration1a(*provider1a);
    EndpointInterfaceRegistration registration1b(*provider1b);

    ASSERT_EQ(registry.Register(registration1a), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId1), provider1a.get());

    // Attempt to register another provider with the same ID
    ASSERT_EQ(registry.Register(registration1b), CHIP_ERROR_DUPLICATE_KEY_ID);
    ASSERT_EQ(registry.Get(kTestEndpointId1), provider1a.get()); // Should still be the first one
}

TEST(TestEndpointInterfaceRegistry, RegisterSameRegistrationObject)
{
    EndpointInterfaceRegistry registry;
    auto build_result = SpanEndpoint::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result));
    auto provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result)));
    ASSERT_NE(provider, nullptr);

    EndpointInterfaceRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Attempt to register the exact same registration object again
    ASSERT_EQ(registry.Register(registration), CHIP_ERROR_DUPLICATE_KEY_ID); // Because registration.next is no longer nullptr
}

TEST(TestEndpointInterfaceRegistry, UnregisterNonExistentProvider)
{
    EndpointInterfaceRegistry registry;
    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR); // Should succeed, no error for non-existent ID

    auto build_result = SpanEndpoint::Builder(kTestEndpointId1)
                            .SetServerClusters(serverClustersArrayWithDescriptor)
                            .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                            .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result));
    auto provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result)));
    ASSERT_NE(provider, nullptr);
    EndpointInterfaceRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Unregister(kTestEndpointId2), CHIP_NO_ERROR); // Different ID. Should still succeed
}

TEST(TestEndpointInterfaceRegistry, GetNonExistentProvider)
{
    EndpointInterfaceRegistry registry;
    ASSERT_EQ(registry.Get(kNonExistentId), nullptr);
}

TEST(TestEndpointInterfaceRegistry, IteratorTest)
{
    EndpointInterfaceRegistry registry;

    auto build_result1 = SpanEndpoint::Builder(kTestEndpointId1)
                             .SetServerClusters(serverClustersArrayWithDescriptor)
                             .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                             .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result1));
    auto provider1 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result1)));

    MinimalServerCluster descriptorCluster2(kTestEndpointId2, chip::app::Clusters::Descriptor::Id);
    std::array<ServerClusterInterface *, 1> serverClustersArrayWithDescriptor2 = { &descriptorCluster2 };

    auto build_result2 = SpanEndpoint::Builder(kTestEndpointId2)
                             .SetServerClusters(serverClustersArrayWithDescriptor2)
                             .SetComposition(DataModel::EndpointCompositionPattern::kTree)
                             .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result2));
    auto provider2 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result2)));

    EndpointInterfaceRegistration registration1(*provider1);
    EndpointInterfaceRegistration registration2(*provider2);

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
    EndpointInterfaceRegistry emptyRegistry;
    ASSERT_EQ(emptyRegistry.begin(), emptyRegistry.end());
}

TEST(TestEndpointInterfaceRegistry, RegisterInvalidArgs)
{
    EndpointInterfaceRegistry registry;
    auto build_result_valid = SpanEndpoint::Builder(kValidIdForArgsTest)
                                  .SetServerClusters(serverClustersArrayWithDescriptor)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result_valid));
    auto providerValid = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result_valid)));
    ASSERT_NE(providerValid, nullptr);

    // Case 2: EndpointInterface returns kInvalidEndpointId
    auto build_result_invalid = SpanEndpoint::Builder(kInvalidEndpointId).Build();
    ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(build_result_invalid)); // Expecting an error for invalid ID
    ASSERT_EQ(std::get<CHIP_ERROR>(build_result_invalid), CHIP_ERROR_INVALID_ARGUMENT);
    // Cannot create a provider from an error, so this part of the test needs adjustment.
    // We'll test that registering a provider that *would* have an invalid ID (if it could be built)
    // is caught by the registry if we somehow bypassed the builder's check.
    // For now, the builder prevents this. If we want to test the registry's check directly,
    // we'd need a mock provider that returns kInvalidEndpointId.
    // The current SpanEndpoint::Builder already prevents creation with kInvalidEndpointId.

    // Case 3: EndpointInterfaceRegistration already part of a list (entry.next != nullptr)
    // To test this, we need two valid providers and registrations.
    MinimalServerCluster descriptorForList1(kListId1ForArgsTest, chip::app::Clusters::Descriptor::Id);
    std::array<ServerClusterInterface *, 1> serverClustersForList1 = { &descriptorForList1 };
    auto build_result_list1                                        = SpanEndpoint::Builder(kListId1ForArgsTest)
                                  .SetServerClusters(serverClustersForList1)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result_list1));
    auto providerForList1 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result_list1)));

    MinimalServerCluster descriptorForList2(kListId2ForArgsTest, chip::app::Clusters::Descriptor::Id);
    std::array<ServerClusterInterface *, 1> serverClustersForList2 = { &descriptorForList2 };
    auto build_result_list2                                        = SpanEndpoint::Builder(kListId2ForArgsTest)
                                  .SetServerClusters(serverClustersForList2)
                                  .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                  .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result_list2));
    auto providerForList2 = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result_list2)));
    EndpointInterfaceRegistration registrationInList1(*providerForList1);
    EndpointInterfaceRegistration registrationInList2(*providerForList2);

    // Manually link to simulate it being in another list
    // This simulates registrationInList2 being part of a list where registrationInList1 is its successor.
    registrationInList2.next = &registrationInList1;
    EXPECT_EQ(registry.Register(registrationInList2), CHIP_ERROR_INVALID_ARGUMENT);
    registrationInList2.next = nullptr; // Reset for other tests or cleanup

    // Test that registering a valid provider after these failures still works
    EndpointInterfaceRegistration registrationValid(*providerValid);
    EXPECT_EQ(registry.Register(registrationValid), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Get(kValidIdForArgsTest), providerValid.get());
}

TEST(TestEndpointInterfaceRegistry, StressTestRegistration)
{
    constexpr int kNumProviders = 100;
    EndpointInterfaceRegistry registry;
    std::vector<std::unique_ptr<SpanEndpoint>> providers_storage; // Owns the providers
    std::list<EndpointInterfaceRegistration> registrations;       // Owns the registration objects, stable addresses
    std::vector<EndpointId> ids;
    std::vector<std::unique_ptr<MinimalServerCluster>> cluster_storage; // Owns the descriptor clusters

    providers_storage.reserve(kNumProviders);
    cluster_storage.reserve(kNumProviders);
    for (int i = 0; i < kNumProviders; ++i)
    {
        EndpointId id = static_cast<EndpointId>(i + 1);
        cluster_storage.push_back(std::make_unique<MinimalServerCluster>(id, chip::app::Clusters::Descriptor::Id));
        std::array<ServerClusterInterface *, 1> serverClusters = { cluster_storage.back().get() };
        auto build_result                                      = SpanEndpoint::Builder(id)
                                .SetServerClusters(serverClusters)
                                .SetComposition(DataModel::EndpointCompositionPattern::kFullFamily)
                                .Build();
        ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_result));
        providers_storage.push_back(std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_result))));
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
            if (reg.endpointInterface->GetEndpointEntry().id == id_to_register)
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
