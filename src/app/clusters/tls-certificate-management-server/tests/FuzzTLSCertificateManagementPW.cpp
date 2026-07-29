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
 *
 *      Three properties, split by which arm of the client-cert flow they reach:
 *        - RootUpsertDoesNotCrash: fuzzed root DER -> 3200-byte buffer lifecycle.
 *        - ClientUpdateDoesNotCrash: fuzzed client DER -> the X.509 parse and the
 *          stored-keypair match check in UpdateClientCertificateEntry.
 *        - ClientIntermediateCertsDoNotCrash: a client cert minted from the keypair
 *          PrepareClientCertificate just stored, so the match check passes and the
 *          fuzzer-controlled intermediateCertificates list drives serialization
 *          across the 31000-byte capacity boundary.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Encode.h>
#include <app/data-model/Nullable.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVTypes.h>
#include <lib/core/TLVWriter.h>
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

constexpr size_t kClientCertCapacity = CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES;
// Room for a TLV array whose payload alone can exceed the persistence capacity.
constexpr size_t kIntermediateScratchBytes = kClientCertCapacity + 2048;
constexpr size_t kMaxIntermediateCount     = 8;

// Generate a valid, self-signed cert carrying keypair's public key (mirrors the
// cluster gtest's GenerateTestCertificate). Gives the mutator a well-formed DER
// shape to perturb, and lets the client-cert property mint a cert whose pubkey
// matches the one CertificateTableImpl has on file.
CHIP_ERROR GenerateSelfSignedCert(Crypto::P256Keypair & keypair, MutableByteSpan & certSpan)
{
    using namespace chip::Credentials;

    ChipDN subjectDN;
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterRCACId(0x1234ABCD));

    // validityStart=1 (just after CHIP epoch), validityEnd=kNullCertTime (9999) → always valid.
    X509CertRequestParams params = { 1, 1, kNullCertTime, subjectDN, subjectDN };
    return NewRootX509Cert(params, keypair, certSpan);
}

// Encode the fuzzer-supplied blobs as a TLV array of octet strings and point list
// at it, matching how the cluster obtains the list: as a DecodableList reading out
// of a decoded ProvisionClientCertificate payload. scratch must outlive list.
CHIP_ERROR SetIntermediateCertificates(const std::vector<std::vector<uint8_t>> & blobs, MutableByteSpan scratch,
                                       app::DataModel::DecodableList<ByteSpan> & list)
{
    TLV::TLVWriter writer;
    writer.Init(scratch.data(), scratch.size());

    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer));
    for (const auto & blob : blobs)
    {
        ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), ByteSpan(blob.data(), blob.size())));
    }
    ReturnErrorOnFailure(writer.EndContainer(outer));
    ReturnErrorOnFailure(writer.Finalize());

    TLV::TLVReader reader;
    reader.Init(scratch.data(), writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());
    return list.Decode(reader);
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
        SuccessOrDie(fx->table.Init(fx->storage));
        SuccessOrDie(fx->table.SetEndpoint(kEndpoint));

        Crypto::P256Keypair keypair;
        uint8_t certBuf[Credentials::kMaxDERCertLength];
        MutableByteSpan certSpan(certBuf);
        if (keypair.Initialize(Crypto::ECPKeyTarget::ECDSA) == CHIP_NO_ERROR &&
            GenerateSelfSignedCert(keypair, certSpan) == CHIP_NO_ERROR)
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

Fixture & ResetFixture()
{
    Fixture & fx = GetFixture();
    fx.storage.ClearStorage();
    SuccessOrDie(fx.table.SetEndpoint(kEndpoint));
    return fx;
}

// FUZZ_TEST 1: fuzzer-controlled root certificate bytes -> the fixed-size
// PersistenceBuffer<3200> serialization copy in UpsertRootCertificateEntry ->
// FabricTableImpl::SetTableEntry. A too-large cert must be rejected by the
// bounded TLV writer, never overflow.
void RootUpsertDoesNotCrash(const std::vector<uint8_t> & certBytes)
{
    Fixture & fx = ResetFixture();

    CertificateTable::RootBuffer buffer;
    Optional<TLSCAID> id; // absent -> allocate a fresh id
    // An error return is a valid outcome here: we only require that the copy path
    // neither crashes nor overflows on fuzzed input.
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
    RETURN_SAFELY_IGNORED fx.table.GetRootCertificateEntry(kFabric, cid, got);
    RETURN_SAFELY_IGNORED fx.table.HasRootCertificateEntry(kFabric, cid);
    uint8_t count = 0;
    RETURN_SAFELY_IGNORED fx.table.GetRootCertificateCount(kFabric, count);

    CertificateTable::RootBuffer iterBuf;
    CertificateTable::BufferedRootCert iterStore(iterBuf);
    RETURN_SAFELY_IGNORED fx.table.IterateRootCertificates(kFabric, iterStore,
                                                           [](CommonIterator<CertificateTable::RootCertStruct> & it) -> CHIP_ERROR {
                                                               CertificateTable::RootCertStruct entry;
                                                               while (it.Next(entry))
                                                               {
                                                               }
                                                               return CHIP_NO_ERROR;
                                                           });
    RETURN_SAFELY_IGNORED fx.table.RemoveRootCertificate(kFabric, cid);
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

// Walk the read-back client entry, including the intermediateCertificates list the
// deserializer re-pointed into the persistence buffer.
void ReadBackClientEntry(Fixture & fx, TLSCCDID cid)
{
    CertificateTable::ClientBuffer getBuf;
    CertificateTable::BufferedClientCert got(getBuf);
    if (fx.table.GetClientCertificateEntry(kFabric, cid, got) == CHIP_NO_ERROR && got.GetCert().intermediateCertificates.HasValue())
    {
        auto iter = got.GetCert().intermediateCertificates.Value().begin();
        while (iter.Next())
        {
            RETURN_SAFELY_IGNORED iter.GetValue().size();
        }
        RETURN_SAFELY_IGNORED iter.GetStatus();
    }
    RETURN_SAFELY_IGNORED fx.table.HasClientCertificateEntry(kFabric, cid);
    uint8_t count = 0;
    RETURN_SAFELY_IGNORED fx.table.GetClientCertificateCount(kFabric, count);

    CertificateTable::ClientBuffer iterBuf;
    CertificateTable::BufferedClientCert iterStore(iterBuf);
    RETURN_SAFELY_IGNORED fx.table.IterateClientCertificates(
        kFabric, iterStore, [](CommonIterator<CertificateTable::ClientCertWithKey> & it) -> CHIP_ERROR {
            CertificateTable::ClientCertWithKey e;
            while (it.Next(e))
            {
            }
            return CHIP_NO_ERROR;
        });
    RETURN_SAFELY_IGNORED fx.table.RemoveClientCertificate(kFabric, cid);
}

// FUZZ_TEST 2: fuzzer-controlled client certificate bytes -> PrepareClientCertificate
// (mints an id + keypair) then UpdateClientCertificateEntry, which runs
// Crypto::ExtractPubkeyFromX509Cert on the supplied cert and compares the extracted
// key against the stored keypair. This property targets those two checks; fuzzed
// bytes cannot carry the freshly-minted public key, so the serialization copy behind
// them is covered by ClientIntermediateCertsDoNotCrash instead.
// clientCertificate is kept PRESENT (the command field is mandatory).
void ClientUpdateDoesNotCrash(const std::vector<uint8_t> & certBytes)
{
    Fixture & fx = ResetFixture();

    CertificateTable::ClientBuffer buffer;
    Optional<TLSCCDID> id; // absent -> PrepareClientCertificate allocates one

    uint8_t csrBuf[Crypto::kMIN_CSR_Buffer_Size];
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

    RETURN_SAFELY_IGNORED fx.table.UpdateClientCertificateEntry(kFabric, id.Value(), buffer, entry);

    // Lifecycle round-trip on the entry Prepare above minted (deserialize path incl.
    // the P256 key CopySpanToMutableSpan), then enumerate + remove.
    ReadBackClientEntry(fx, id.Value());
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

// FUZZ_TEST 3: reach the client-cert serialization copy. PrepareClientCertificate
// mints and stores a keypair; we read it back, mint a self-signed cert carrying its
// public key, and hand that to UpdateClientCertificateEntry so the pubkey-match check
// passes. The fuzzer then controls the intermediateCertificates list, i.e. the bulk of
// what SetTableEntry serializes into PersistenceBuffer<31000>. Blob sizes span that
// capacity, so both the accepted-write and the bounded-TLVWriter rejection are covered.
void ClientIntermediateCertsDoNotCrash(const std::vector<std::vector<uint8_t>> & intermediates)
{
    Fixture & fx = ResetFixture();

    CertificateTable::ClientBuffer buffer;
    Optional<TLSCCDID> id;

    uint8_t csrBuf[Crypto::kMIN_CSR_Buffer_Size];
    uint8_t sigBuf[Crypto::kMax_ECDSA_Signature_Length];
    MutableByteSpan csr(csrBuf);
    MutableByteSpan sig(sigBuf);
    if (fx.table.PrepareClientCertificate(kFabric, ByteSpan(kNonce), buffer, id, csr, sig) != CHIP_NO_ERROR || !id.HasValue())
    {
        return;
    }

    CertificateTable::ClientBuffer keyBuf;
    CertificateTable::BufferedClientCert stored(keyBuf);
    if (fx.table.GetClientCertificateEntry(kFabric, id.Value(), stored) != CHIP_NO_ERROR)
    {
        return;
    }

    Crypto::P256Keypair keypair;
    uint8_t certBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuf);
    if (keypair.Deserialize(stored.mCertWithKey.key) != CHIP_NO_ERROR || GenerateSelfSignedCert(keypair, certSpan) != CHIP_NO_ERROR)
    {
        return;
    }

    ClientCertStruct entry;
    entry.ccdid = id.Value();
    entry.clientCertificate.SetValue(chip::app::DataModel::MakeNullable(ByteSpan(certSpan)));

    // scratch backs the list's TLV reader for as long as entry is in use.
    std::vector<uint8_t> scratch(kIntermediateScratchBytes);
    app::DataModel::DecodableList<ByteSpan> list;
    if (SetIntermediateCertificates(intermediates, MutableByteSpan(scratch.data(), scratch.size()), list) == CHIP_NO_ERROR)
    {
        entry.intermediateCertificates.SetValue(list);
    }

    RETURN_SAFELY_IGNORED fx.table.UpdateClientCertificateEntry(kFabric, id.Value(), buffer, entry);

    ReadBackClientEntry(fx, id.Value());
}

std::vector<std::vector<std::vector<uint8_t>>> IntermediateSeeds()
{
    return {
        {},                                                                     // no intermediates: the plain accepted-write path
        { std::vector<uint8_t>(100, 0x30) },                                    // one small blob, comfortably under capacity
        { std::vector<uint8_t>(kClientCertCapacity + 100, 0x30) },              // single blob past the 31000-byte buffer
        std::vector<std::vector<uint8_t>>(8, std::vector<uint8_t>(3800, 0x30)), // 30400 bytes: just under
        std::vector<std::vector<uint8_t>>(8, std::vector<uint8_t>(4000, 0x30)), // 32000 bytes: just over
    };
}

FUZZ_TEST(FuzzTLSCertificateManagementPW, ClientIntermediateCertsDoNotCrash)
    .WithDomains(VectorOf(VectorOf(Arbitrary<uint8_t>()).WithMaxSize(kClientCertCapacity + 256))
                     .WithMaxSize(kMaxIntermediateCount)
                     .WithSeeds(&IntermediateSeeds));

} // namespace
