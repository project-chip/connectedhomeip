/*
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
#pragma once

#include <access/SubjectDescriptor.h>
#include <app/AttributeEncodeState.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

#include <vector>

namespace chip {
namespace app {
namespace Testing {

/// Contains information about a single parsed item inside an attribute data IB
struct DecodedAttributeData
{
    chip::DataVersion dataVersion;
    chip::app::ConcreteDataAttributePath attributePath;
    chip::TLV::TLVReader dataReader;

    CHIP_ERROR DecodeFrom(const chip::app::AttributeDataIB::Parser & parser);
};

/// Maintains an internal TLV buffer for data encoding and
/// decoding for ReportIBs.
///
/// Main use case is that explicit TLV layouts (structure and container starting) need to be
/// prepared to have a proper AttributeReportIBs::Builder/parser to exist.
class EncodedReportIBs
{
public:
    /// Initialize the report structures required to encode a
    CHIP_ERROR StartEncoding(app::AttributeReportIBs::Builder & builder);
    CHIP_ERROR FinishEncoding(app::AttributeReportIBs::Builder & builder);

    /// Decode the embedded attribute report IBs.
    /// The TLVReaders inside data have a lifetime tied to the current object (its readers point
    /// inside the current object)
    CHIP_ERROR Decode(std::vector<DecodedAttributeData> & decoded_items) const;

private:
    constexpr static size_t kMaxTLVBufferSize = 1024;

    uint8_t mTlvDataBuffer[kMaxTLVBufferSize];
    TLV::TLVType mOuterStructureType;
    TLV::TLVWriter mEncodeWriter;
    ByteSpan mDecodeSpan;
};

/// Contains a `ReadAttributeRequest` as well as classes to convert this into a AttributeReportIBs
/// and later decode it
///
/// It wraps boilerplate code to obtain a `AttributeValueEncoder` as well as later decoding
/// the underlying encoded data for verification.
///
/// Usage:
///
///    ReadOperation operation(1 /* endpoint */, 2 /* cluster */, 3 /* attribute */);
///
///    auto encoder = operation.StartEncoding(/* ... */);
///    ASSERT_NE(encoder.get(), nullptr);
///
///    // use encoder, like pass in to a WriteAttribute() call
///
///    ASSERT_EQ(operation.FinishEncoding(), CHIP_NO_ERROR);
///
///    // extract the written data
///    std::vector<DecodedAttributeData> items;
///    ASSERT_EQ(read_request.GetEncodedIBs().Decode(items), CHIP_NO_ERROR);
///
///    // can use items::dataReader for a chip::TLV::TLVReader access to the underlying data
///    // for example
///    uint32_t value = 0;
///    chip::TLV::TLVReader reader(items[0].dataReader);
///    ASSERT_EQ(reader.Get(value), CHIP_NO_ERROR);
///    ASSERT_EQ(value, 123u);
///
///
class ReadOperation
{
public:
    /// Represents parameters for StartEncoding
    class EncodingParams
    {
    public:
        EncodingParams() {}

        EncodingParams & SetDataVersion(chip::DataVersion v)
        {
            mDataVersion = v;
            return *this;
        }

        EncodingParams & SetIsFabricFiltered(bool filtered)
        {
            mIsFabricFiltered = filtered;
            return *this;
        }

        EncodingParams & SetEncodingState(const AttributeEncodeState & state)
        {
            mAttributeEncodeState = state;
            return *this;
        }

        chip::DataVersion GetDataVersion() const { return mDataVersion; }
        bool GetIsFabricFiltered() const { return mIsFabricFiltered; }
        const AttributeEncodeState & GetAttributeEncodeState() const { return mAttributeEncodeState; }

    private:
        chip::DataVersion mDataVersion = 0x1234;
        bool mIsFabricFiltered         = false;
        AttributeEncodeState mAttributeEncodeState;
    };

    ReadOperation(const ConcreteAttributePath & path)
    {
        mRequest.path              = path;
        mRequest.subjectDescriptor = kDenySubjectDescriptor;
    }

    ReadOperation(EndpointId endpoint, ClusterId cluster, AttributeId attribute) :
        ReadOperation(ConcreteAttributePath(endpoint, cluster, attribute))
    {}

    ReadOperation & SetSubjectDescriptor(const chip::Access::SubjectDescriptor & descriptor)
    {
        VerifyOrDie(mState == State::kInitializing);
        mRequest.subjectDescriptor = descriptor;
        return *this;
    }

    ReadOperation & SetReadFlags(const BitFlags<DataModel::ReadFlags> & flags)
    {
        VerifyOrDie(mState == State::kInitializing);
        mRequest.readFlags = flags;
        return *this;
    }

    ReadOperation & SetOperationFlags(const BitFlags<DataModel::OperationFlags> & flags)
    {
        VerifyOrDie(mState == State::kInitializing);
        mRequest.operationFlags = flags;
        return *this;
    }

    ReadOperation & SetPathExpanded(bool value)
    {
        VerifyOrDie(mState == State::kInitializing);
        mRequest.path.mExpanded = value;
        return *this;
    }

    /// Start the encoding of a new element with the given data version associated to it.
    ///
    /// The input attribute encoding state will be attached to the returned value encoded (so that
    /// encoding for list elements is possible)
    ///
    std::unique_ptr<AttributeValueEncoder> StartEncoding(const EncodingParams & params = EncodingParams());

    /// Completes the encoding and finalizes the undelying AttributeReport.
    ///
    /// Call this to finish a set of `StartEncoding` values and have access to
    /// the underlying `GetEncodedIBs`
    CHIP_ERROR FinishEncoding();

    /// Get the underlying read request (i.e. path and flags) for this request
    const DataModel::ReadAttributeRequest & GetRequest() const { return mRequest; }

    /// Once encoding has finished, you can get access to the underlying
    /// written data via GetEncodedIBs.
    const EncodedReportIBs & GetEncodedIBs() const
    {
        VerifyOrDie(mState == State::kFinished);
        return mEncodedIBs;
    }

private:
    enum class State
    {
        kInitializing, // Setting up initial values (i.e. setting up mRequest)
        kEncoding,     // Encoding values via StartEncoding
        kFinished,     // FinishEncoding has been called

    };
    State mState = State::kInitializing;

    DataModel::ReadAttributeRequest mRequest;

    // encoded-used classes
    EncodedReportIBs mEncodedIBs;
    AttributeReportIBs::Builder mAttributeReportIBsBuilder;
};

} // namespace Testing
} // namespace app
} // namespace chip
