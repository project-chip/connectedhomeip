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

#include <app/server-cluster/testing/MockCommandHandler.h>

#include <app/data-model/Encode.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVTags.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Testing {

CHIP_ERROR MockCommandHandler::FallibleAddStatus(const app::ConcreteCommandPath & aRequestCommandPath,
                                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                                 const char * context)
{
    mStatuses.emplace_back(StatusRecord{ aRequestCommandPath, aStatus, context });
    return CHIP_NO_ERROR;
}

void MockCommandHandler::AddStatus(const app::ConcreteCommandPath & aRequestCommandPath,
                                   const Protocols::InteractionModel::ClusterStatusCode & aStatus, const char * context)
{
    CHIP_ERROR err = FallibleAddStatus(aRequestCommandPath, aStatus, context);
    VerifyOrDie(err == CHIP_NO_ERROR);
}

CHIP_ERROR MockCommandHandler::AddResponseData(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                                               const app::DataModel::EncodableToTLV & aEncodable)
{
    chip::System::PacketBufferHandle handle = chip::MessagePacketBuffer::New(chip::kMaxAppMessageLen);
    VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter baseWriter;
    baseWriter.Init(handle->Start(), handle->MaxDataLength());
    app::DataModel::FabricAwareTLVWriter writer(baseWriter, mSubjectDescriptor.fabricIndex);
    TLV::TLVType ct;

    ReturnErrorOnFailure(writer.mTLVWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ct));
    ReturnErrorOnFailure(aEncodable.EncodeTo(writer, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));
    ReturnErrorOnFailure(writer.mTLVWriter.EndContainer(ct));
    handle->SetDataLength(writer.mTLVWriter.GetLengthWritten());

    mResponses.emplace_back(ResponseRecord{ aResponseCommandId, std::move(handle), aRequestCommandPath });
    return CHIP_NO_ERROR;
}

CHIP_ERROR MockCommandHandler::AddResponseData(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                                               const EncodableResponsePayload & aPayload)
{
    struct PayloadAdapter : public app::DataModel::EncodableToTLV
    {
        const EncodableResponsePayload & mPayload;
        PayloadAdapter(const EncodableResponsePayload & payload) : mPayload(payload) {}
        CHIP_ERROR EncodeTo(app::DataModel::FabricAwareTLVWriter & writer, TLV::Tag tag) const override
        {
            return mPayload.EncodeTo(writer, tag);
        }
        CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override
        {
            app::DataModel::FabricAwareTLVWriter fabricWriter(writer, kUndefinedFabricIndex);
            return mPayload.EncodeTo(fabricWriter, tag);
        }
    };
    PayloadAdapter adapter(aPayload);
    return AddResponseData(aRequestCommandPath, aResponseCommandId, adapter);
}

void MockCommandHandler::AddResponse(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                                     const app::DataModel::EncodableToTLV & aEncodable)
{
    CHIP_ERROR err = AddResponseData(aRequestCommandPath, aResponseCommandId, aEncodable);
    VerifyOrDie(err == CHIP_NO_ERROR);
}

void MockCommandHandler::AddResponse(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                                     const EncodableResponsePayload & aPayload)
{
    CHIP_ERROR err = AddResponseData(aRequestCommandPath, aResponseCommandId, aPayload);
    VerifyOrDie(err == CHIP_NO_ERROR);
}

CHIP_ERROR MockCommandHandler::GetResponseReader(TLV::TLVReader & reader) const
{
    return GetResponseReader(reader, 0);
}

CHIP_ERROR MockCommandHandler::GetResponseReader(TLV::TLVReader & reader, size_t index) const
{
    VerifyOrReturnError(!mResponses.empty(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(index < mResponses.size(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mResponses[index].encodedData.IsNull(), CHIP_ERROR_INCORRECT_STATE);

    reader.Init(mResponses[index].encodedData->Start(), mResponses[index].encodedData->DataLength());
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerContainer;
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer));
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));

    return CHIP_NO_ERROR;
}

} // namespace Testing
} // namespace chip
