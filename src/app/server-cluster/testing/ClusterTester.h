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
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/List.h>
#include <app/data-model/NullObject.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <credentials/FabricTable.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace chip {
namespace Testing {

/**
 * @brief Helper class to manage the lifecycle of a Fabric for testing.
 *
 * This encapsulates the logic required to initialize a FabricTable,
 * create a mock Fabric, and manage related certificate storage.
 * It is designed to be used by ClusterTester or a test fixture.
 */
class FabricTestHelper
{
public:
    FabricTestHelper(PersistentStorageDelegate * storage) : mStorage(storage), mRootCertSpan(mRootCertDER), mNocSpan(mNocDER)
    {
        // Zero-initialize the buffers to satisfy compilers treating this as an error.
        memset(mRootCertDER, 0, sizeof(mRootCertDER)); // <-- ADD THIS
        memset(mNocDER, 0, sizeof(mNocDER));           // <-- ADD THIS
    }

    /**
     * @brief Initializes the Fabric table and adds a new test fabric.
     *
     * @param fabricIndexOut Output parameter for the newly created FabricIndex.
     * @return CHIP_ERROR
     */
    CHIP_ERROR SetUpFabric(FabricIndex & fabricIndexOut);

    /**
     * @brief Tears down the created fabric and cleans up storage.
     *
     * @param fabricIndex The FabricIndex to tear down.
     * @return CHIP_ERROR
     */
    CHIP_ERROR TearDownFabric(FabricIndex fabricIndex);

    FabricTable & GetFabricTable() { return mfabricTable; }

private:
    CHIP_ERROR SetUpCertificates();

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

CHIP_ERROR FabricTestHelper::SetUpFabric(FabricIndex & fabricIndexOut)
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

    ReturnErrorOnFailure(mfabricTable.CommitPendingFabricData());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTestHelper::TearDownFabric(FabricIndex fabricIndex)
{
    if (fabricIndex != chip::kUndefinedFabricIndex)
    {
        // Credentials::SetGroupDataProvider(nullptr); // Done in the test fixture's TearDown
        // GroupDataProviderImpl::RemoveFabric is usually done separately
        // However, we just need to ensure the fabric is deleted from FabricTable and storage is cleaned.

        CHIP_ERROR err = CHIP_NO_ERROR;

        if ((err = mOpCertStore.RemoveOpCertsForFabric(fabricIndex)) != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "TearDownFabric: RemoveOpCertsForFabric failed: %s", err.AsString());
        }
        if ((err = mfabricTable.Delete(fabricIndex)) != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "TearDownFabric: Delete fabric failed: %s", err.AsString());
        }

        mfabricTable.Shutdown();
        mOpCertStore.Finish();
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR FabricTestHelper::SetUpCertificates()
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
    ReturnErrorOnFailure(rootRequestParams.IssuerDN.AddAttribute(chip::ASN1::kOID_AttributeType_CommonName,
                                                                 CharSpan(rootName, strlen(rootName)), true /* isPrintableString */
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

    ReturnErrorOnFailure(chip::Credentials::ConvertX509CertToChipCert(nocDERSpan, mNocSpan));

    return CHIP_NO_ERROR;
}

// Helper class for testing clusters.
//
// This class ensures that data read by attribute is referencing valid memory for all
// read requests until the ClusterTester object goes out of scope. (for the case where the underlying read references a list or
// string that points to TLV data).
//
// Read/Write of all attribute types should work, but make sure to use ::Type for encoding
// and ::DecodableType for decoding structure types.
//
// Example of usage:
//
// ExampleCluster cluster(someEndpointId);
//
// // Possibly steps to setup the cluster
//
// ClusterTester tester(cluster);
// app::Clusters::ExampleCluster::Attributes::FeatureMap::TypeInfo::DecodableType features;
// ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
//
// app::Clusters::ExampleCluster::Attributes::ExampleListAttribute::TypeInfo::DecodableType list;
// ASSERT_EQ(tester.ReadAttribute(LabelList::Id, list), CHIP_NO_ERROR);
// auto it = list.begin();
// while (it.Next())
// {
//     ASSERT_GT(it.GetValue().label.size(), 0u);
// }
//
class ClusterTester
{
public:
    ClusterTester(app::ServerClusterInterface & cluster) :
        mCluster(cluster), mFabricHelper(&mTestServerClusterContext.StorageDelegate())
    {}
    app::ServerClusterContext & GetServerClusterContext() { return mTestServerClusterContext.Get(); }

    // Read attribute into `out` parameter.
    // The `out` parameter must be of the correct type for the attribute being read.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::DecodableType` for the `out` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename T>
    app::DataModel::ActionReturnStatus ReadAttribute(AttributeId attr_id, T & out)
    {
        VerifyOrReturnError(VerifyClusterPathsValid(), CHIP_ERROR_INCORRECT_STATE);
        auto path = mCluster.GetPaths()[0];

        // Store the read operation in a vector<std::unique_ptr<...>> to ensure its lifetime
        // using std::unique_ptr because ReadOperation is non-copyable and non-movable
        // vector reallocation is not an issue since we store unique_ptrs
        std::unique_ptr<chip::Testing::ReadOperation> readOperation =
            std::make_unique<chip::Testing::ReadOperation>(path.mEndpointId, path.mClusterId, attr_id);

        mReadOperations.push_back(std::move(readOperation));
        chip::Testing::ReadOperation & readOperationRef = *mReadOperations.back().get();

        std::unique_ptr<app::AttributeValueEncoder> encoder = readOperationRef.StartEncoding();
        app::DataModel::ActionReturnStatus status           = mCluster.ReadAttribute(readOperationRef.GetRequest(), *encoder);
        VerifyOrReturnError(status.IsSuccess(), status);
        ReturnErrorOnFailure(readOperationRef.FinishEncoding());

        std::vector<chip::Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(readOperationRef.GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        return app::DataModel::Decode(attributeData[0].dataReader, out);
    }

    // Write attribute from `value` parameter.
    // The `value` parameter must be of the correct type for the attribute being written.
    // Use `app::Clusters::<ClusterName>::Attributes::<AttributeName>::TypeInfo::Type` for the `value` parameter to be spec
    // compliant (see the comment of the class for usage example).
    // Will construct the attribute path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename T>
    app::DataModel::ActionReturnStatus WriteAttribute(AttributeId attr, const T & value)
    {
        const auto & paths = mCluster.GetPaths();

        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        app::ConcreteAttributePath path(paths[0].mEndpointId, paths[0].mClusterId, attr);
        chip::Testing::WriteOperation writeOp(path);

        // Create a stable object on the stack
        Access::SubjectDescriptor subjectDescriptor{ mHandler.GetAccessingFabricIndex() };
        writeOp.SetSubjectDescriptor(subjectDescriptor);

        uint8_t buffer[1024];
        TLV::TLVWriter writer;
        writer.Init(buffer);

        // - DataModel::Encode(integral, enum, etc.) for simple types.
        // - DataModel::Encode(List<X>) for lists (which iterates and calls Encode on elements).
        // - DataModel::Encode(Struct) for non-fabric-scoped structs.
        // - Note: For attribute writes, DataModel::EncodeForWrite is usually preferred for fabric-scoped types,
        //         but the generic DataModel::Encode often works as a top-level function.
        //         If you use EncodeForWrite, you ensure fabric-scoped list items are handled correctly:

        if constexpr (app::DataModel::IsFabricScoped<T>::value)
        {
            ReturnErrorOnFailure(chip::app::DataModel::EncodeForWrite(writer, TLV::AnonymousTag(), value));
        }
        else
        {
            ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, TLV::AnonymousTag(), value));
        }

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOp.GetRequest().subjectDescriptor);

        return mCluster.WriteAttribute(writeOp.GetRequest(), decoder);
    }

    // Result structure for Invoke operations, containing both status and decoded response.
    template <typename ResponseType>
    struct InvokeResult
    {
        std::optional<app::DataModel::ActionReturnStatus> status;
        std::optional<ResponseType> response;

        // Returns true if the command was successful and response is available
        bool IsSuccess() const
        {
            if constexpr (std::is_same_v<ResponseType, app::DataModel::NullObjectType>)
                return status.has_value() && status->IsSuccess();
            else
                return status.has_value() && status->IsSuccess() && response.has_value();
        }
    };

    // Invoke a command and return the decoded result.
    // The `request` parameter must be of the correct type for the command being invoked.
    // Use `app::Clusters::<ClusterName>::Commands::<CommandName>::Type` for the `request` parameter to be spec compliant
    // Will construct the command path using the first path returned by `GetPaths()` on the cluster.
    // @returns `CHIP_ERROR_INCORRECT_STATE` if `GetPaths()` doesn't return a list with one path.
    template <typename RequestType, typename ResponseType = typename RequestType::ResponseType>
    [[nodiscard]] InvokeResult<ResponseType> Invoke(chip::CommandId commandId, const RequestType & request)
    {
        InvokeResult<ResponseType> result;

        const auto & paths = mCluster.GetPaths();
        VerifyOrReturnValue(paths.size() == 1u, result);

        mHandler.ClearResponses();
        mHandler.ClearStatuses();

        const app::DataModel::InvokeRequest invokeRequest = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

        TLV::TLVWriter writer;
        writer.Init(mTlvBuffer);

        TLV::TLVReader reader;

        VerifyOrReturnValue(request.Encode(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);
        VerifyOrReturnValue(writer.Finalize() == CHIP_NO_ERROR, result);

        reader.Init(mTlvBuffer, writer.GetLengthWritten());
        VerifyOrReturnValue(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()) == CHIP_NO_ERROR, result);

        result.status = mCluster.InvokeCommand(invokeRequest, reader, &mHandler);

        // If InvokeCommand returned nullopt, it means the command implementation handled the response.
        // We need to check the mock handler for a data response or a status response.
        if (!result.status.has_value())
        {
            if (mHandler.HasResponse())
            {
                // A data response was added, so the command is successful.
                result.status = app::DataModel::ActionReturnStatus(CHIP_NO_ERROR);
            }
            else if (mHandler.HasStatus())
            {
                // A status response was added. Use the last one.
                result.status = app::DataModel::ActionReturnStatus(mHandler.GetLastStatus().status);
            }
            else
            {
                // Neither response nor status was provided; this is unexpected.
                // This would happen either in error (as mentioned here) or if the command is supposed
                // to be handled asynchronously. ClusterTester does not support such asynchronous processing.
                result.status = app::DataModel::ActionReturnStatus(CHIP_ERROR_INCORRECT_STATE);
                ChipLogError(
                    Test, "InvokeCommand returned nullopt, but neither HasResponse nor HasStatus is true. Setting error status.");
            }
        }

        // If command was successful and there's a response, decode it (skip for NullObjectType)
        if constexpr (!std::is_same_v<ResponseType, app::DataModel::NullObjectType>)
        {
            if (result.status.has_value() && result.status->IsSuccess() && mHandler.HasResponse())
            {
                ResponseType decodedResponse;
                CHIP_ERROR decodeError = mHandler.DecodeResponse(decodedResponse);
                if (decodeError == CHIP_NO_ERROR)
                {
                    result.response = std::move(decodedResponse);
                }
                else
                {
                    // Decode failed; reflect error in status and log
                    result.status = app::DataModel::ActionReturnStatus(decodeError);
                    ChipLogError(Test, "DecodeResponse failed: %s", decodeError.AsString());
                }
            }
        }

        return result;
    }

    // convenience method: most requests have a `GetCommandId` (and GetClusterId() as well).
    template <typename RequestType, typename ResponseType = typename RequestType::ResponseType>
    [[nodiscard]] InvokeResult<ResponseType> Invoke(const RequestType & request)
    {
        return Invoke(RequestType::GetCommandId(), request);
    }

    // Returns the next generated event from the event generator in the test server cluster context
    std::optional<LogOnlyEvents::EventInformation> GetNextGeneratedEvent()
    {
        return mTestServerClusterContext.EventsGenerator().GetNextEvent();
    }

    std::vector<app::AttributePathParams> & GetDirtyList() { return mTestServerClusterContext.ChangeListener().DirtyList(); }

    void SetFabricIndex(FabricIndex fabricIndex) { mHandler.SetFabricIndex(fabricIndex); }

    CHIP_ERROR SetUpMockFabric()
    {
        FabricIndex currentHandlerIndex = mHandler.GetAccessingFabricIndex();

        CHIP_ERROR err = mFabricHelper.SetUpFabric(currentHandlerIndex);

        if (err == CHIP_NO_ERROR)
        {
            SetFabricIndex(currentHandlerIndex);
        }
        return err;
    }

    CHIP_ERROR TearDownMockFabric()
    {
        FabricIndex currentFabricIndex = mHandler.GetAccessingFabricIndex();

        CHIP_ERROR err = mFabricHelper.TearDownFabric(currentFabricIndex);

        SetFabricIndex(chip::kUndefinedFabricIndex);

        return err;
    }

    FabricTestHelper & GetFabricHelper() { return mFabricHelper; }

private:
    bool VerifyClusterPathsValid()
    {
        auto paths = mCluster.GetPaths();
        if (paths.size() != 1)
        {
            ChipLogError(Test, "cluster.GetPaths() did not return exactly one path");
            return false;
        }
        return true;
    }

    TestServerClusterContext mTestServerClusterContext{};
    app::ServerClusterInterface & mCluster;

    // Buffer size for TLV encoding/decoding of command payloads.
    // 256 bytes was chosen as a conservative upper bound for typical command payloads in tests.
    // All command payloads used in tests must fit within this buffer; tests with larger payloads will fail.
    // If protocol or test requirements change, this value may need to be increased.
    static constexpr size_t kTlvBufferSize = 256;

    chip::Testing::MockCommandHandler mHandler;
    uint8_t mTlvBuffer[kTlvBufferSize];
    std::vector<std::unique_ptr<ReadOperation>> mReadOperations;

    FabricTestHelper mFabricHelper;
};

} // namespace Testing
} // namespace chip
