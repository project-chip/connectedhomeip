#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <algorithm>
#include <array>
#include <list>
#include <random>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

constexpr EndpointId kTestEndpointId1    = 1;
constexpr EndpointId kTestEndpointId2    = 2;
constexpr EndpointId kNonExistentId      = 3;
constexpr EndpointId kValidIdForArgsTest = 1; // Same as kTestEndpointId1, but for clarity in RegisterInvalidArgs
constexpr EndpointId kListId1ForArgsTest = 10;
constexpr EndpointId kListId2ForArgsTest = 11;

} // namespace

TEST(TestEndpointInterfaceRegistry, CreateAndDestroy)
{
    EndpointInterfaceRegistry registry;

    // Create an endpoint
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    ASSERT_NE(endpoint, nullptr);

    EndpointInterface * endpoint_ptr = endpoint.get(); // Save raw pointer for comparison

    // Register it
    EndpointInterfaceRegistration registration(*endpoint, { .id = kTestEndpointId1 });
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Check if it can be retrieved
    ASSERT_EQ(registry.Get(kTestEndpointId1), endpoint_ptr);

    // Unregister the endpoint
    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR);

    // Destroy the endpoint
    endpoint.reset();

    // Check if it is no longer retrievable
    // After unregistering, Get(kTestEndpointId1) should return nullptr.
    ASSERT_EQ(registry.Get(kTestEndpointId1), nullptr);
}

TEST(TestEndpointInterfaceRegistry, RegisterMultipleProviders)
{
    EndpointInterfaceRegistry registry;

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    ASSERT_NE(endpoint1, nullptr);
    ASSERT_NE(endpoint2, nullptr);

    EndpointInterfaceRegistration registration1(*endpoint1, { .id = kTestEndpointId1 });
    EndpointInterfaceRegistration registration2(*endpoint2, { .id = kTestEndpointId2 });

    ASSERT_EQ(registry.Register(registration1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Register(registration2), CHIP_NO_ERROR);

    ASSERT_EQ(registry.Get(kTestEndpointId1), endpoint1.get());
    ASSERT_EQ(registry.Get(kTestEndpointId2), endpoint2.get());

    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId1), nullptr);
    ASSERT_EQ(registry.Get(kTestEndpointId2), endpoint2.get());

    ASSERT_EQ(registry.Unregister(kTestEndpointId2), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId2), nullptr);
}

TEST(TestEndpointInterfaceRegistry, RegisterDuplicateProviderId)
{
    EndpointInterfaceRegistry registry;

    auto endpoint1a = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpoint1b = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build()); // Same ID

    ASSERT_NE(endpoint1a, nullptr);
    ASSERT_NE(endpoint1b, nullptr);

    EndpointInterfaceRegistration registration1a(*endpoint1a, { .id = kTestEndpointId1 });
    EndpointInterfaceRegistration registration1b(*endpoint1b, { .id = kTestEndpointId1 });

    ASSERT_EQ(registry.Register(registration1a), CHIP_NO_ERROR);
    ASSERT_EQ(registry.Get(kTestEndpointId1), endpoint1a.get());

    // Attempt to register another endpoint with the same ID
    ASSERT_EQ(registry.Register(registration1b), CHIP_ERROR_DUPLICATE_KEY_ID);
    ASSERT_EQ(registry.Get(kTestEndpointId1), endpoint1a.get()); // Should still be the first one
}

TEST(TestEndpointInterfaceRegistry, RegisterSameRegistrationObject)
{
    EndpointInterfaceRegistry registry;
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    ASSERT_NE(endpoint, nullptr);

    EndpointInterfaceRegistration registration(*endpoint, { .id = kTestEndpointId1 });
    ASSERT_EQ(registry.Register(registration), CHIP_NO_ERROR);

    // Attempt to register the exact same registration object again
    ASSERT_EQ(registry.Register(registration), CHIP_ERROR_DUPLICATE_KEY_ID);
}

TEST(TestEndpointInterfaceRegistry, UnregisterNonExistentProvider)
{
    EndpointInterfaceRegistry registry;
    ASSERT_EQ(registry.Unregister(kTestEndpointId1), CHIP_ERROR_NOT_FOUND);
}

TEST(TestEndpointInterfaceRegistry, GetNonExistentProvider)
{
    EndpointInterfaceRegistry registry;
    ASSERT_EQ(registry.Get(kNonExistentId), nullptr);
}

TEST(TestEndpointInterfaceRegistry, IteratorTest)
{
    EndpointInterfaceRegistry registry;

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    EndpointInterfaceRegistration registration1(*endpoint1, { .id = kTestEndpointId1 });
    EndpointInterfaceRegistration registration2(*endpoint2, { .id = kTestEndpointId2 });

    EXPECT_SUCCESS(registry.Register(registration1));
    EXPECT_SUCCESS(registry.Register(registration2)); // Registry stores in LIFO order for simple list prepend

    size_t count = 0;
    bool found1  = false;
    bool found2  = false;
    for (auto & reg : registry)
    {
        ASSERT_NE(reg.endpointInterface, nullptr);
        if (reg.endpointInterface == endpoint1.get())
        {
            found1 = true;
        }
        if (reg.endpointInterface == endpoint2.get())
        {
            found2 = true;
        }
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
    auto endpointValid = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    ASSERT_NE(endpointValid, nullptr);

    // Case 2: EndpointInterface returns kInvalidEndpointId
    EndpointInterfaceRegistration registrationInvalidId(*endpointValid, { .id = kInvalidEndpointId });
    ASSERT_EQ(registry.Register(registrationInvalidId), CHIP_ERROR_INVALID_ARGUMENT);

    // Case 3: EndpointInterfaceRegistration already part of a list (entry.next != nullptr)
    // To test this, we need two valid endpoints and registrations.
    auto endpointForList1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpointForList2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    EndpointInterfaceRegistration registrationInList1(*endpointForList1, { .id = kListId1ForArgsTest });
    EndpointInterfaceRegistration registrationInList2(*endpointForList2, { .id = kListId2ForArgsTest });

    // Manually link to simulate it being in another list
    // This simulates registrationInList2 being part of a list where registrationInList1 is its successor.
    registrationInList2.next = &registrationInList1;
    EXPECT_EQ(registry.Register(registrationInList2), CHIP_ERROR_INVALID_ARGUMENT);
    registrationInList2.next = nullptr; // Reset for other tests or cleanup

    // Test that registering a valid endpoint after these failures still works
    EndpointInterfaceRegistration registrationValid(*endpointValid, { .id = kValidIdForArgsTest });
    EXPECT_EQ(registry.Register(registrationValid), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Get(kValidIdForArgsTest), endpointValid.get());
}

TEST(TestEndpointInterfaceRegistry, StressTestRegistration)
{
    constexpr int kNumProviders = 100;
    EndpointInterfaceRegistry registry;
    std::vector<std::unique_ptr<SpanEndpoint>> endpoints_storage; // Owns the endpoints
    std::list<EndpointInterfaceRegistration> registrations;       // Owns the registration objects, stable addresses
    std::vector<EndpointId> ids;

    endpoints_storage.reserve(kNumProviders);
    for (int i = 0; i < kNumProviders; ++i)
    {
        EndpointId id = static_cast<EndpointId>(i + 1);
        endpoints_storage.push_back(std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build()));
        ids.push_back(id);
        registrations.emplace_back(*endpoints_storage.back(), DataModel::EndpointEntry{ .id = id });
    }

    // Register all
    auto reg_it = registrations.begin();
    for (size_t i = 0; i < static_cast<size_t>(kNumProviders); ++i, ++reg_it)
    {
        ASSERT_EQ(registry.Register(*reg_it), CHIP_NO_ERROR) << "Failed to register endpoint with ID " << ids[i];
    }

    // Verify all can be retrieved
    std::vector<EndpointId> shuffled_ids = ids;
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), std::default_random_engine(0));
    for (EndpointId id : shuffled_ids)
    {
        ASSERT_NE(registry.Get(id), nullptr) << "Failed to get endpoint with ID " << id;
    }

    // Unregister all in shuffled order
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), std::default_random_engine(1));
    for (EndpointId id : shuffled_ids)
    {
        ASSERT_EQ(registry.Unregister(id), CHIP_NO_ERROR) << "Failed to unregister endpoint with ID " << id;
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
            if (reg.endpointEntry.id == id_to_register)
            {
                ASSERT_EQ(registry.Register(reg), CHIP_NO_ERROR) << "Failed to re-register endpoint with ID " << id_to_register;
                found_and_registered = true;
                break;
            }
        }
        ASSERT_TRUE(found_and_registered);
    }

    for (EndpointId id : ids)
    {
        ASSERT_NE(registry.Get(id), nullptr) << "Failed to get endpoint with ID " << id << " after re-registration";
    }
}
