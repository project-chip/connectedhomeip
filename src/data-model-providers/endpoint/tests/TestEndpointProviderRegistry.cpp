/*
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

#include <app/data-model-provider/MetadataTypes.h>
#include <data-model-providers/endpoint/EndpointProviderRegistry.h>
#include <data-model-providers/endpoint/SpanEndpointProvider.h>

#include <algorithm> // For std::shuffle
#include <list>      // For std::list in stress test
#include <random>    // For std::default_random_engine
#include <vector>    // For std::vector in stress test
using namespace chip;
using namespace chip::app;

TEST(TestEndpointProviderRegistry, CreateAndDestroy)
{
    EndpointProviderRegistry registry;

    // Create a provider
    auto build_pair_provider =
        SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair_provider.second, CHIP_NO_ERROR);
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(build_pair_provider.first));
    ASSERT_NE(provider, nullptr);
    EndpointProviderInterface * provider_ptr = provider.get(); // Save raw pointer for comparison

    // Register it
    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Check if it can be retrieved
    ASSERT_EQ(registry.Get(1), provider_ptr);

    // Unregister the provider
    ASSERT_EQ(registry.Unregister(1), CHIP_NO_ERROR);

    // Destroy the provider
    provider.reset();

    // Check if it is no longer retrievable
    // After unregistering, Get(1) should return nullptr.
    ASSERT_EQ(registry.Get(1), nullptr);
}

TEST(TestEndpointProviderRegistry, RegisterMultipleProviders)
{
    EndpointProviderRegistry registry;

    auto build_pair1 = SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair1.second, CHIP_NO_ERROR);
    auto provider1 = std::make_unique<SpanEndpointProvider>(std::move(build_pair1.first));

    auto build_pair2 = SpanEndpointProvider::Builder(2).SetComposition(DataModel::EndpointCompositionPattern::kTree).build();
    ASSERT_EQ(build_pair2.second, CHIP_NO_ERROR);
    auto provider2 = std::make_unique<SpanEndpointProvider>(std::move(build_pair2.first));
    ASSERT_NE(provider1, nullptr);
    ASSERT_NE(provider2, nullptr);

    EndpointProviderRegistration registration1(*provider1);
    EndpointProviderRegistration registration2(*provider2);

    ASSERT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Register(registration2), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Get(1), provider1.get());
    ASSERT_EQ(registry.Get(2), provider2.get());
    ASSERT_EQ(registry.Count(), 2u);

    ASSERT_EQ(registry.Unregister(1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(1), nullptr);
    ASSERT_EQ(registry.Get(2), provider2.get());
    ASSERT_EQ(registry.Count(), 1u);

    ASSERT_EQ(registry.Unregister(2), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(2), nullptr);
    ASSERT_EQ(registry.Count(), 0u);
}

TEST(TestEndpointProviderRegistry, RegisterDuplicateProviderId)
{
    EndpointProviderRegistry registry;

    auto build_pair1a = SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair1a.second, CHIP_NO_ERROR);
    auto provider1a = std::make_unique<SpanEndpointProvider>(std::move(build_pair1a.first));

    auto build_pair1b =
        SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kTree).build(); // Same ID
    ASSERT_EQ(build_pair1b.second, CHIP_NO_ERROR);
    auto provider1b = std::make_unique<SpanEndpointProvider>(std::move(build_pair1b.first));
    ASSERT_NE(provider1a, nullptr);
    ASSERT_NE(provider1b, nullptr);

    EndpointProviderRegistration registration1a(*provider1a);
    EndpointProviderRegistration registration1b(*provider1b);

    ASSERT_EQ(registry.Register(registration1a), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(1), provider1a.get());

    // Attempt to register another provider with the same ID
    ASSERT_EQ(registry.Register(registration1b), CHIP_ERROR_DUPLICATE_KEY_ID);
    ASSERT_EQ(registry.Get(1), provider1a.get()); // Should still be the first one
    ASSERT_EQ(registry.Count(), 1u);
}

TEST(TestEndpointProviderRegistry, RegisterSameRegistrationObject)
{
    EndpointProviderRegistry registry;
    auto build_pair = SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair.second, CHIP_NO_ERROR);
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(build_pair.first));
    ASSERT_NE(provider, nullptr);

    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Attempt to register the exact same registration object again
    ASSERT_EQ(registry.Register(registration), CHIP_ERROR_DUPLICATE_KEY_ID); // Because registration.next is no longer nullptr
    ASSERT_EQ(registry.Count(), 1u);
}

TEST(TestEndpointProviderRegistry, UnregisterNonExistentProvider)
{
    EndpointProviderRegistry registry;
    ASSERT_EQ(registry.Unregister(1), CHIP_ERROR_NOT_FOUND);

    auto build_pair = SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair.second, CHIP_NO_ERROR);
    auto provider = std::make_unique<SpanEndpointProvider>(std::move(build_pair.first));
    ASSERT_NE(provider, nullptr);
    EndpointProviderRegistration registration(*provider);
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Unregister(2), CHIP_ERROR_NOT_FOUND); // Different ID
    ASSERT_EQ(registry.Count(), 1u);
}

TEST(TestEndpointProviderRegistry, GetNonExistentProvider)
{
    EndpointProviderRegistry registry;
    ASSERT_EQ(registry.Get(1), nullptr);
}

TEST(TestEndpointProviderRegistry, IteratorTest)
{
    EndpointProviderRegistry registry;

    auto build_pair1 = SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair1.second, CHIP_NO_ERROR);
    auto provider1 = std::make_unique<SpanEndpointProvider>(std::move(build_pair1.first));

    auto build_pair2 = SpanEndpointProvider::Builder(2).SetComposition(DataModel::EndpointCompositionPattern::kTree).build();
    ASSERT_EQ(build_pair2.second, CHIP_NO_ERROR);
    auto provider2 = std::make_unique<SpanEndpointProvider>(std::move(build_pair2.first));
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
        if (ep->GetEndpointEntry().id == 1)
            found1 = true;
        if (ep->GetEndpointEntry().id == 2)
            found2 = true;
        count++;
    }
    ASSERT_EQ(count, 2u);
    ASSERT_TRUE(found1);
    ASSERT_TRUE(found2);

    // Test empty iteration
    EndpointProviderRegistry emptyRegistry;
    count = 0;
    for (auto * ep : emptyRegistry)
    {
        (void) ep; // unused
        count++;
    }
    ASSERT_EQ(count, 0u);
}

TEST(TestEndpointProviderRegistry, RegisterInvalidArgs)
{
    EndpointProviderRegistry registry;
    auto build_pair_valid =
        SpanEndpointProvider::Builder(1).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair_valid.second, CHIP_NO_ERROR);
    auto providerValid = std::make_unique<SpanEndpointProvider>(std::move(build_pair_valid.first));
    ASSERT_NE(providerValid, nullptr);

    // Case 2: EndpointProviderInterface returns kInvalidEndpointId
    auto build_pair_invalid = SpanEndpointProvider::Builder(kInvalidEndpointId).build();
    ASSERT_NE(build_pair_invalid.second, CHIP_NO_ERROR); // Expecting an error for invalid ID
    auto providerInvalidId = std::make_unique<SpanEndpointProvider>(std::move(build_pair_invalid.first));
    ASSERT_NE(providerInvalidId, nullptr);
    ASSERT_EQ(providerInvalidId->GetEndpointEntry().id, kInvalidEndpointId);
    EndpointProviderRegistration registrationInvalidId(*providerInvalidId);
    EXPECT_EQ(registry.Register(registrationInvalidId), CHIP_ERROR_INVALID_ARGUMENT);

    // Case 3: EndpointProviderRegistration already part of a list (entry.next != nullptr)
    // To test this, we need two valid providers and registrations.
    auto build_pair_list1 =
        SpanEndpointProvider::Builder(10).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair_list1.second, CHIP_NO_ERROR);
    auto providerForList1 = std::make_unique<SpanEndpointProvider>(std::move(build_pair_list1.first));
    auto build_pair_list2 =
        SpanEndpointProvider::Builder(11).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
    ASSERT_EQ(build_pair_list2.second, CHIP_NO_ERROR);
    auto providerForList2 = std::make_unique<SpanEndpointProvider>(std::move(build_pair_list2.first));
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
    EXPECT_EQ(registry.Count(), 1u);
    EXPECT_EQ(registry.Get(1), providerValid.get());
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
        EndpointId id = static_cast<EndpointId>(i + 1);
        auto build_pair =
            SpanEndpointProvider::Builder(id).SetComposition(DataModel::EndpointCompositionPattern::kFullFamily).build();
        ASSERT_EQ(build_pair.second, CHIP_NO_ERROR);
        providers_storage.push_back(std::make_unique<SpanEndpointProvider>(std::move(build_pair.first)));
        ids.push_back(id);
        registrations.emplace_back(*providers_storage.back()); // Create registration from the provider
    }

    // Register all
    auto reg_it = registrations.begin();
    for (size_t i = 0; i < static_cast<size_t>(kNumProviders); ++i, ++reg_it)
    {
        ASSERT_EQ(registry.Register(*reg_it), CHIP_NO_ERROR) << "Failed to register provider with ID " << ids[i];
    }
    ASSERT_EQ(registry.Count(), static_cast<size_t>(kNumProviders));

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
    ASSERT_EQ(registry.Count(), 0u);

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
    ASSERT_EQ(registry.Count(), static_cast<size_t>(kNumProviders));
    for (EndpointId id : ids)
    {
        ASSERT_NE(registry.Get(id), nullptr) << "Failed to get provider with ID " << id << " after re-registration";
    }
}
