/*
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 *    @file
 *      FuzzTest harness for the ICD Management cluster command handlers.
 *
 *      The three commands the cluster accepts all arrive as decoded structs, so
 *      the fuzzer drives the decoded fields and lets ClusterTester encode them
 *      rather than mutating raw TLV -- otherwise nearly every input is spent
 *      failing to decode instead of exercising the handlers.
 *
 *      The field worth the most attention is RegisterClient's `key`: it is a
 *      caller-sized ByteSpan that ends up in the monitoring table's fixed-size
 *      symmetric key storage, so the domain deliberately straddles the expected
 *      16-byte length in both directions.
 *
 *      RegisterClient and UnregisterClient fork on whether the invoking subject
 *      has administrator privileges: an admin may act on an existing entry
 *      outright, while a non-admin must present a matching verificationKey. The
 *      access-control delegate here grants nothing, so CheckAdmin resolves every
 *      subject to non-admin and it is the verificationKey comparison against
 *      stored key material that gets driven. Reaching the administrator arm would
 *      need access-control entries granting that privilege.
 *
 *      Each property runs against a table that persists across inputs, so entries
 *      left by earlier iterations are what later ones find, update, and evict --
 *      including at the per-fabric capacity limit.
 *
 *      Oracle: no sanitizer error, and the handler must always return a status
 *      rather than leaving the table in a state that trips a later read-back.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <access/AccessControl.h>
#include <access/examples/ExampleAccessControlDelegate.h>
#include <app/clusters/icd-management-server/ICDManagementCluster.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/IcdManagement/Commands.h>
#include <clusters/IcdManagement/Enums.h>
#include <clusters/IcdManagement/Metadata.h>
#include <crypto/CryptoBuildConfig.h>
#include <crypto/DefaultSessionKeystore.h>
#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/Constants.h>
#include <lib/support/logging/TextOnlyLogging.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace fuzztest;

using chip::Testing::ClusterTester;
using chip::Testing::FabricTestFixture;
using chip::Testing::TestServerClusterContext;

constexpr EndpointId kEndpoint = kRootEndpointId;

class HarnessDeviceTypeResolver : public Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId, EndpointId) override { return false; }
};

HarnessDeviceTypeResolver gDeviceTypeResolver;

struct Fixture
{
    Crypto::DefaultSessionKeystore keystore;
    TestServerClusterContext context;
    FabricTestFixture fabricFixture{ &context.StorageDelegate() };
    // SetUpTestFabric takes this as an in/out parameter and rejects the
    // undefined index, so it must name a concrete fabric to create.
    FabricIndex fabricIndex = 1;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    ICDManagementClusterWithCIP cluster;
#else
    ICDManagementCluster cluster;
#endif

    Fixture() :
        cluster(kEndpoint, keystore, fabricFixture.GetFabricTable(), ICDConfigurationData::GetInstance(), OptionalAttributeSet(0),
                ICDManagementCluster::OptionalCommandSet().Set<IcdManagement::Commands::StayActiveRequest::Id>(),
                BitMask<IcdManagement::UserActiveModeTriggerBitmap>(0), CharSpan())
    {}
};

Fixture * gFixture = nullptr;

Fixture & GetFixture()
{
    static std::once_flag once;
    std::call_once(once, [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        // Fuzz binaries use gmock_main and so miss the unit-test main's PSA
        // initialization; the backend must be initialized before any crypto runs.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        // The cluster logs per command; at fuzzing rates that dominates runtime
        // and output volume both.
        Logging::SetLogFilter(Logging::kLogCategory_None);

        // RegisterClient/UnregisterClient gate on CheckAdmin, which consults the
        // global AccessControl. Uninitialized, Check() returns neither success nor
        // ACCESS_DENIED, so both handlers bail at their second guard and never
        // reach the monitoring table at all.
        VerifyOrDie(Access::GetAccessControl().Init(Access::Examples::GetAccessControlDelegate(), gDeviceTypeResolver) ==
                    CHIP_NO_ERROR);

        auto * fx = new Fixture();
        VerifyOrDie(fx->fabricFixture.SetUpTestFabric(fx->fabricIndex) == CHIP_NO_ERROR);
        VerifyOrDie(fx->cluster.Startup(fx->context.Get()) == CHIP_NO_ERROR);

        gFixture = fx;
        // Teardown via atexit rather than a static destructor: the cluster and
        // fabric table assert on their own shutdown state, and running that
        // during static destruction aborts the process after the run has already
        // finished -- which looks like a crash and loses the coverage profile.
        std::atexit([] {
            if (gFixture != nullptr)
            {
                gFixture->cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
                gFixture->fabricFixture.TearDownTestFabric(gFixture->fabricIndex);
                delete gFixture;
                gFixture = nullptr;
            }
            Access::GetAccessControl().Finish();
            Platform::MemoryShutdown();
        });
    });
    return *gFixture;
}

#if CHIP_CONFIG_ENABLE_ICD_CIP

// The registration key is a caller-sized span copied into fixed-size key
// storage, so the sizes either side of 16 are the ones that matter.
std::vector<std::vector<uint8_t>> KeySeeds()
{
    return {
        {},
        std::vector<uint8_t>(1, 0xAA),
        std::vector<uint8_t>(15, 0xAA),
        std::vector<uint8_t>(16, 0xAA),
        std::vector<uint8_t>(17, 0xAA),
        std::vector<uint8_t>(32, 0xAA),
        std::vector<uint8_t>(64, 0xAA),
    };
}

void RegisterClientDoesNotCrash(uint64_t checkInNodeID, uint64_t monitoredSubject, const std::vector<uint8_t> & key,
                                uint8_t clientTypeRaw, const std::vector<uint8_t> & verificationKey, bool sendVerificationKey)
{
    Fixture & fx = GetFixture();

    ClusterTester tester(fx.cluster, &fx.fabricFixture);
    tester.SetFabricIndex(fx.fabricIndex);

    Commands::RegisterClient::Type request;
    request.checkInNodeID    = checkInNodeID;
    request.monitoredSubject = monitoredSubject;
    request.key              = ByteSpan(key.data(), key.size());
    request.clientType       = static_cast<ClientTypeEnum>(clientTypeRaw);
    if (sendVerificationKey)
    {
        request.verificationKey.SetValue(ByteSpan(verificationKey.data(), verificationKey.size()));
    }

    auto result = tester.Invoke(Commands::RegisterClient::Id, request);
    // ClusterTester returns no status when it could not encode the request --
    // which a large fuzzer-chosen key can cause -- so that case is not a handler
    // defect. When the command did reach the handler it must not come back
    // without having produced a response or a status.
    if (result.status.has_value())
    {
        EXPECT_NE(result.status.value(), app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE));
    }
}

void UnregisterClientDoesNotCrash(uint64_t checkInNodeID, const std::vector<uint8_t> & verificationKey, bool sendVerificationKey)
{
    Fixture & fx = GetFixture();

    ClusterTester tester(fx.cluster, &fx.fabricFixture);
    tester.SetFabricIndex(fx.fabricIndex);

    // Each property runs as its own process, so nothing has registered a client
    // yet and every unregistration would return NotFound. Register first, with a
    // key the fuzzer also controls, so the lookup succeeds and the key
    // verification and removal paths are reachable.
    {
        Commands::RegisterClient::Type seed;
        seed.checkInNodeID    = checkInNodeID;
        seed.monitoredSubject = checkInNodeID;
        seed.key              = ByteSpan(verificationKey.data(), verificationKey.size());
        seed.clientType       = ClientTypeEnum::kPermanent;
        auto seedResult       = tester.Invoke(Commands::RegisterClient::Id, seed);
        (void) seedResult;
    }

    Commands::UnregisterClient::Type request;
    request.checkInNodeID = checkInNodeID;
    if (sendVerificationKey)
    {
        request.verificationKey.SetValue(ByteSpan(verificationKey.data(), verificationKey.size()));
    }

    auto result = tester.Invoke(Commands::UnregisterClient::Id, request);
    // ClusterTester returns no status when it could not encode the request --
    // which a large fuzzer-chosen key can cause -- so that case is not a handler
    // defect. When the command did reach the handler it must not come back
    // without having produced a response or a status.
    if (result.status.has_value())
    {
        EXPECT_NE(result.status.value(), app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE));
    }
}

FUZZ_TEST(ICDManagementClusterPW, RegisterClientDoesNotCrash)
    // The monitoring table holds CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC (2)
    // entries per fabric, so unconstrained 64-bit node IDs fill it within two
    // inputs and every later one returns ResourceExhausted without registering.
    // A small set makes registrations collide, which is what reaches the
    // existing-entry path and its verificationKey comparison.
    .WithDomains(ElementOf<uint64_t>({ 0x1234, 0x1235, 0x1236 }), Arbitrary<uint64_t>(),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(KeySeeds()).WithMaxSize(128),
                 // Only 0 and 1 are valid ClientTypeEnum values; anything else is
                 // rejected by the first guard in the handler. Left unconstrained,
                 // ~254/256 inputs return before the entry is ever looked up.
                 ElementOf<uint8_t>({ 0, 1, 2, 3, 0xFF }), Arbitrary<std::vector<uint8_t>>().WithSeeds(KeySeeds()).WithMaxSize(128),
                 Arbitrary<bool>());

FUZZ_TEST(ICDManagementClusterPW, UnregisterClientDoesNotCrash)
    // Same node-ID set as RegisterClient, so unregistration finds an entry
    // rather than returning NotFound on almost every input.
    .WithDomains(ElementOf<uint64_t>({ 0x1234, 0x1235, 0x1236 }), Arbitrary<std::vector<uint8_t>>().WithSeeds(KeySeeds()).WithMaxSize(128),
                 Arbitrary<bool>());

#endif // CHIP_CONFIG_ENABLE_ICD_CIP

void StayActiveRequestDoesNotCrash(uint32_t stayActiveDuration)
{
    Fixture & fx = GetFixture();

    ClusterTester tester(fx.cluster, &fx.fabricFixture);
    tester.SetFabricIndex(fx.fabricIndex);

    Commands::StayActiveRequest::Type request;
    request.stayActiveDuration = stayActiveDuration;

    auto result = tester.Invoke(Commands::StayActiveRequest::Id, request);
    // ClusterTester returns no status when it could not encode the request --
    // which a large fuzzer-chosen key can cause -- so that case is not a handler
    // defect. When the command did reach the handler it must not come back
    // without having produced a response or a status.
    if (result.status.has_value())
    {
        EXPECT_NE(result.status.value(), app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE));
    }
}

FUZZ_TEST(ICDManagementClusterPW, StayActiveRequestDoesNotCrash)
    .WithDomains(Arbitrary<uint32_t>().WithSeeds({ 0u, 1u, 30u, 60u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu }));

} // namespace
