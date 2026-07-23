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

/**
 *    @file
 *      FuzzTest harness for the TlsCertificateManagement server's storage engine
 *      (chip::app::Clusters::Tls::CertificateTableImpl).
 *
 *      Exercises the path where variable-length DER certificate bytes are copied
 *      into the fixed-size persistence buffers
 *      (PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES=3200> /
 *      <...CLIENT_CERT_BYTES=31000>) via FabricTableImpl::SetTableEntry, and where
 *      the public key is extracted from the supplied X.509 cert
 *      (Crypto::ExtractPubkeyFromX509Cert).
 *
 *      The cluster's own gtest scaffold (TestTLSCertificateManagementCluster.cpp)
 *      uses an all-mock CertificateTable whose UpsertRootCertificateEntry /
 *      UpdateClientCertificateEntry return CHIP_NO_ERROR without touching a real
 *      buffer, so it cannot reach the serialization copy. This harness stands up the
 *      real CertificateTableImpl backed by a TestPersistentStorageDelegate and drives
 *      the copy path with fuzzer-controlled cert bytes. Build+run under ASan (+UBSan)
 *      via the chip_pw_fuzztest toolchain.
 */

#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/data-model/Nullable.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters::Tls;
using namespace fuzztest;

using ClientCertStruct = CertificateTable::ClientCertStruct;

constexpr EndpointId kEndpoint = 1;
constexpr FabricIndex kFabric  = 1;
constexpr uint8_t kNonce[32]   = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20 };

// Generate a valid, self-signed root cert for use as a seed (mirrors the cluster
// gtest's GenerateTestCertificate). Gives the mutator a well-formed DER shape to
// perturb so the ExtractPubkey / parse arms get real gradient.
CHIP_ERROR GenerateValidCert(MutableByteSpan & certSpan)
{
    using namespace chip::Credentials;
    using namespace chip::Crypto;

    P256Keypair keypair;
    ReturnErrorOnFailure(keypair.Initialize(ECPKeyTarget::ECDSA));

    ChipDN subjectDN;
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterRCACId(0x1234ABCD));

    // validityStart=1 (just after CHIP epoch), validityEnd=kNullCertTime (9999) → always valid.
    X509CertRequestParams params = { 1, 1, kNullCertTime, subjectDN, subjectDN };
    return NewRootX509Cert(params, keypair, certSpan);
}

// One-time, leaked, reused-across-inputs harness state (FuzzTest re-invokes the
// property many times; the storage delegate is cleared per input in the property).
struct Fixture
{
    TestPersistentStorageDelegate storage;
    CertificateTableImpl table;
    std::vector<uint8_t> validCertSeed;
};

Fixture * gFixture = nullptr;

Fixture & GetFixture()
{
    static std::once_flag once;
    std::call_once(once, [] {
        // MemoryInit must run before any CHIP object is constructed.
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        auto * fx = new Fixture();
        VerifyOrDie(fx->table.Init(fx->storage) == CHIP_NO_ERROR);
        VerifyOrDie(fx->table.SetEndpoint(kEndpoint) == CHIP_NO_ERROR);

        uint8_t certBuf[Credentials::kMaxDERCertLength];
        MutableByteSpan certSpan(certBuf);
        if (GenerateValidCert(certSpan) == CHIP_NO_ERROR)
        {
            fx->validCertSeed.assign(certSpan.begin(), certSpan.end());
        }

        gFixture = fx;
        std::atexit([] {
            if (gFixture != nullptr)
            {
                // Destroy the fixture (runs Finish + frees members) before shutting the
                // allocator down, so no outstanding allocation trips leak detection.
                delete gFixture;
                gFixture = nullptr;
            }
            Platform::MemoryShutdown();
        });
    });
    return *gFixture;
}

// FUZZ_TEST 1: fuzzer-controlled root certificate bytes -> the fixed-size
// PersistenceBuffer<3200> serialization copy in UpsertRootCertificateEntry ->
// FabricTableImpl::SetTableEntry. A too-large cert must be rejected by the
// bounded TLV writer, never overflow.
void RootUpsertDoesNotCrash(const std::vector<uint8_t> & certBytes)
{
    Fixture & fx = GetFixture();
    fx.storage.ClearStorage();
    VerifyOrDie(fx.table.SetEndpoint(kEndpoint) == CHIP_NO_ERROR);

    CertificateTable::RootBuffer buffer;
    Optional<TLSCAID> id; // absent -> allocate a fresh id
    // Return value intentionally ignored: we only care that the copy path does not
    // crash / overflow on fuzzed input; an error return is a valid outcome.
    if (fx.table.UpsertRootCertificateEntry(kFabric, id, buffer, ByteSpan(certBytes.data(), certBytes.size())) != CHIP_NO_ERROR ||
        !id.HasValue())
    {
        return;
    }

    // Lifecycle round-trip: read the provisioned cert back out (deserialize
    // path), enumerate, then remove. Exercises Get/Has/Count/Iterate/Remove, none of
    // which the write-only path reaches.
    const TLSCAID cid = id.Value();
    CertificateTable::RootBuffer getBuf;
    CertificateTable::BufferedRootCert got(getBuf);
    (void) fx.table.GetRootCertificateEntry(kFabric, cid, got);
    (void) fx.table.HasRootCertificateEntry(kFabric, cid);
    uint8_t count = 0;
    (void) fx.table.GetRootCertificateCount(kFabric, count);

    CertificateTable::RootBuffer iterBuf;
    CertificateTable::BufferedRootCert iterStore(iterBuf);
    (void) fx.table.IterateRootCertificates(kFabric, iterStore,
                                            [](CommonIterator<CertificateTable::RootCertStruct> & it) -> CHIP_ERROR {
                                                CertificateTable::RootCertStruct entry;
                                                while (it.Next(entry))
                                                {
                                                }
                                                return CHIP_NO_ERROR;
                                            });
    (void) fx.table.RemoveRootCertificate(kFabric, cid);
}

std::vector<std::vector<uint8_t>> RootSeeds()
{
    Fixture & fx = GetFixture();
    std::vector<std::vector<uint8_t>> seeds;
    if (!fx.validCertSeed.empty())
    {
        seeds.push_back(fx.validCertSeed);
    }
    seeds.push_back({});                               // empty
    seeds.push_back(std::vector<uint8_t>(3300, 0xAB)); // just over the 3200-byte buffer
    return seeds;
}

FUZZ_TEST(FuzzTLSCertificateManagementPW, RootUpsertDoesNotCrash)
    .WithDomains(VectorOf(Arbitrary<uint8_t>()).WithMaxSize(4096).WithSeeds(&RootSeeds));

// FUZZ_TEST 2: fuzzer-controlled client certificate bytes -> PrepareClientCertificate
// (mints an id + keypair) then UpdateClientCertificateEntry, which runs
// Crypto::ExtractPubkeyFromX509Cert on the supplied cert and then serializes into
// PersistenceBuffer<31000>. clientCertificate is kept PRESENT (the command field is
// mandatory).
void ClientUpdateDoesNotCrash(const std::vector<uint8_t> & certBytes)
{
    Fixture & fx = GetFixture();
    fx.storage.ClearStorage();
    VerifyOrDie(fx.table.SetEndpoint(kEndpoint) == CHIP_NO_ERROR);

    CertificateTable::ClientBuffer buffer;
    Optional<TLSCCDID> id; // absent -> PrepareClientCertificate allocates one

    uint8_t csrBuf[1024];
    uint8_t sigBuf[Crypto::kMax_ECDSA_Signature_Length];
    MutableByteSpan csr(csrBuf);
    MutableByteSpan sig(sigBuf);
    if (fx.table.PrepareClientCertificate(kFabric, ByteSpan(kNonce), buffer, id, csr, sig) != CHIP_NO_ERROR || !id.HasValue())
    {
        return;
    }

    ClientCertStruct entry;
    entry.ccdid = id.Value();
    entry.clientCertificate.SetValue(chip::app::DataModel::MakeNullable(ByteSpan(certBytes.data(), certBytes.size())));
    // intermediateCertificates left default (empty list).

    (void) fx.table.UpdateClientCertificateEntry(kFabric, id.Value(), buffer, entry);

    // Lifecycle round-trip on the client entry the Prepare above minted (deserialize
    // path incl. the P256 key CopySpanToMutableSpan), then enumerate + remove.
    const TLSCCDID cid = id.Value();
    CertificateTable::ClientBuffer getBuf;
    CertificateTable::BufferedClientCert got(getBuf);
    (void) fx.table.GetClientCertificateEntry(kFabric, cid, got);
    (void) fx.table.HasClientCertificateEntry(kFabric, cid);
    uint8_t count = 0;
    (void) fx.table.GetClientCertificateCount(kFabric, count);

    CertificateTable::ClientBuffer iterBuf;
    CertificateTable::BufferedClientCert iterStore(iterBuf);
    (void) fx.table.IterateClientCertificates(kFabric, iterStore,
                                              [](CommonIterator<CertificateTable::ClientCertWithKey> & it) -> CHIP_ERROR {
                                                  CertificateTable::ClientCertWithKey e;
                                                  while (it.Next(e))
                                                  {
                                                  }
                                                  return CHIP_NO_ERROR;
                                              });
    (void) fx.table.RemoveClientCertificate(kFabric, cid);
}

std::vector<std::vector<uint8_t>> ClientSeeds()
{
    Fixture & fx = GetFixture();
    std::vector<std::vector<uint8_t>> seeds;
    if (!fx.validCertSeed.empty())
    {
        seeds.push_back(fx.validCertSeed);
    }
    seeds.push_back({}); // empty present cert -> ExtractPubkey rejects
    return seeds;
}

FUZZ_TEST(FuzzTLSCertificateManagementPW, ClientUpdateDoesNotCrash)
    .WithDomains(VectorOf(Arbitrary<uint8_t>()).WithMaxSize(4096).WithSeeds(&ClientSeeds));

} // namespace
