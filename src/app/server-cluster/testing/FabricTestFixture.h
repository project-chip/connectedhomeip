/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once
#include <credentials/FabricTable.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

#include <memory>
#include <optional>
#include <vector>

namespace chip {
namespace Testing {
/**
 * @brief Fixture class to manage the lifecycle of a Fabric for testing.
 *
 * This encapsulates the logic required to initialize a FabricTable,
 * create a mock Fabric, and manage related certificate storage.
 * It is designed to be used by ClusterTester or a test fixture but not mandatory.
 */
class FabricTestFixture
{
public:
    FabricTestFixture(PersistentStorageDelegate * storage) :
        mStorage(storage), mRootCertSpan(mRootCertDER, sizeof(mRootCertDER)), mNocSpan(mNocDER, sizeof(mNocDER))
    {}

    /**
     * @brief Initializes the Fabric table and adds a new test fabric.
     *
     * @param fabricIndexOut Output parameter for the newly created FabricIndex.
     * @return CHIP_ERROR
     */

    CHIP_ERROR SetUpTestFabric(FabricIndex & fabricIndexOut)
    {
        ReturnErrorOnFailure(mOpCertStore.Init(mStorage));
        initParams.opCertStore         = &mOpCertStore;
        initParams.storage             = mStorage;
        initParams.operationalKeystore = nullptr; // Use default
        ReturnErrorOnFailure(mfabricTable.Init(initParams));

        ReturnErrorOnFailure(mfabricTable.SetFabricIndexForNextAddition(fabricIndexOut));
        ReturnErrorOnFailure(SetUpCertificates());

        CHIP_ERROR err = mfabricTable.AddNewFabricForTest(
            mRootCertSpan, ByteSpan(), mNocSpan, ByteSpan(mSerializedOpKey.Bytes(), mSerializedOpKey.Length()), &fabricIndexOut);
        ReturnErrorOnFailure(err);

        return mfabricTable.CommitPendingFabricData();
    }

    /**
     * @brief Tears down the created fabric and cleans up storage.
     *
     * @param fabricIndex The FabricIndex to tear down.
     * @return CHIP_ERROR
     */
    CHIP_ERROR TearDownTestFabric(FabricIndex & fabricIndex)
    {
        VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

        // Credentials::SetGroupDataProvider(nullptr); // Done in the test fixture's TearDown
        // GroupDataProviderImpl::RemoveFabric is usually done separately
        // However, we just need to ensure the fabric is deleted from FabricTable and storage is cleaned.

        CHIP_ERROR err = CHIP_NO_ERROR;

        if ((err = mOpCertStore.RemoveOpCertsForFabric(fabricIndex)) != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "TearDownFabric: RemoveOpCertsForFabric failed: %s", err.Format());
        }
        if ((err = mfabricTable.Delete(fabricIndex)) != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "TearDownFabric: Delete fabric failed: %s", err.Format());
        }

        mfabricTable.Shutdown();
        mOpCertStore.Finish();

        return CHIP_NO_ERROR;
    }

    FabricTable & GetFabricTable() { return mfabricTable; }

private:
    CHIP_ERROR SetUpCertificates()
    {
        Crypto::P256Keypair rootCACredentials;
        ReturnErrorOnFailure(rootCACredentials.Initialize(Crypto::ECPKeyTarget::ECDSA));

        Crypto::P256Keypair deviceOpKey;
        ReturnErrorOnFailure(deviceOpKey.Initialize(Crypto::ECPKeyTarget::ECDSA));

        ReturnErrorOnFailure(deviceOpKey.Serialize(mSerializedOpKey));

        // Create temporary X.509 (DER) buffers
        uint8_t rootCertDER_temp[chip::Credentials::kMaxDERCertLength];
        MutableByteSpan rootCertDERSpan(rootCertDER_temp);

        uint8_t nocDER_temp[chip::Credentials::kMaxDERCertLength];
        MutableByteSpan nocDERSpan(nocDER_temp);

        mRootCertSpan = MutableByteSpan(mRootCertDER);
        mNocSpan      = MutableByteSpan(mNocDER);

        chip::Credentials::X509CertRequestParams rootRequestParams;
        rootRequestParams.SerialNumber  = kRootCertSerial;
        rootRequestParams.ValidityStart = kCertValidityStart;
        rootRequestParams.ValidityEnd   = kRootCertValidityDurationSeconds;

        const char * rootName = "My Test Root CA";
        ReturnErrorOnFailure(rootRequestParams.IssuerDN.AddAttribute(
            chip::ASN1::kOID_AttributeType_CommonName, CharSpan(rootName, strlen(rootName)), true /* isPrintableString */
            ));
        ReturnErrorOnFailure(rootRequestParams.IssuerDN.AddAttribute(chip::ASN1::kOID_AttributeType_MatterRCACId, kTestRcacId));
        rootRequestParams.SubjectDN = rootRequestParams.IssuerDN;

        ReturnErrorOnFailure(chip::Credentials::NewRootX509Cert(rootRequestParams, rootCACredentials, rootCertDERSpan));

        // Convert X.509 DER to Matter TLV
        ReturnErrorOnFailure(chip::Credentials::ConvertX509CertToChipCert(rootCertDERSpan, mRootCertSpan));

        chip::Credentials::X509CertRequestParams nocRequestParams;
        nocRequestParams.SerialNumber  = kNocSerial;
        nocRequestParams.ValidityStart = kCertValidityStart;
        nocRequestParams.ValidityEnd   = kNocCertValidityDurationSeconds;
        nocRequestParams.IssuerDN      = rootRequestParams.SubjectDN;
        ReturnErrorOnFailure(nocRequestParams.SubjectDN.AddAttribute(chip::ASN1::kOID_AttributeType_MatterFabricId, kTestFabricId));
        ReturnErrorOnFailure(nocRequestParams.SubjectDN.AddAttribute(chip::ASN1::kOID_AttributeType_MatterNodeId, kTestNodeId));
        ReturnErrorOnFailure(
            chip::Credentials::NewNodeOperationalX509Cert(nocRequestParams, deviceOpKey.Pubkey(), rootCACredentials, nocDERSpan));

        return chip::Credentials::ConvertX509CertToChipCert(nocDERSpan, mNocSpan);
    }

    PersistentStorageDelegate * mStorage;

    // Certificates and keys
    uint8_t mRootCertDER[chip::Credentials::kMaxDERCertLength];
    MutableByteSpan mRootCertSpan;
    uint8_t mNocDER[chip::Credentials::kMaxDERCertLength];
    MutableByteSpan mNocSpan;
    Crypto::P256SerializedKeypair mSerializedOpKey;

    // Fabric-related storage
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    FabricTable mfabricTable;
    FabricTable::InitParams initParams;

    // Test constants (copied from test fixture)
    static constexpr FabricId kTestFabricId                    = 0xDEADBEEF00000001;
    static constexpr NodeId kTestNodeId                        = 0xDEADBEEF00000002;
    static constexpr uint64_t kTestRcacId                      = 0x1111222233334444;
    static constexpr uint64_t kRootCertSerial                  = 1;
    static constexpr uint64_t kNocSerial                       = 2;
    static constexpr uint64_t kCertValidityStart               = 0;
    static constexpr uint32_t kRootCertValidityDurationSeconds = 315360000; // 10 years
    static constexpr uint32_t kNocCertValidityDurationSeconds  = 31536000;  // 1 year
};

} // namespace Testing
} // namespace chip
