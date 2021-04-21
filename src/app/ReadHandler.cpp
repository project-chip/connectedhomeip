/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines read handler for a CHIP Interaction Data model
 *
 */

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPath.h>
#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>

namespace chip {
namespace app {
CHIP_ERROR ReadHandler::Init(InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(apDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    mpExchangeCtx     = nullptr;
    mpDelegate        = apDelegate;
    mSuppressResponse = true;
    mGetToAllEvents   = true;
    ClearClusterInfo();
    MoveToState(HandlerState::Initialized);

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadHandler::Shutdown()
{
    ReleaseClusterInfoList();
    ClearExistingExchangeContext();
    MoveToState(HandlerState::Uninitialized);
    mpDelegate = nullptr;
    ClearClusterInfo();
}

CHIP_ERROR ReadHandler::ClearExistingExchangeContext()
{
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadHandler::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    mpExchangeCtx = apExchangeContext;

    err = ProcessReadRequest(std::move(aPayload));
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogFunctError(err);
        Shutdown();
    }

    return err;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
exit:
    ChipLogFunctError(err);
    Shutdown();
    return err;
}

CHIP_ERROR ReadHandler::ProcessReadRequest(System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    ReadRequest::Parser readRequestParser;
    EventPathList::Parser eventPathListParser;
    AttributePathList::Parser attributePathListParser;
    TLV::TLVReader eventPathListReader;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = readRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = readRequestParser.GetAttributePathList(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
        ProcessAttributePathList(attributePathListParser);
    }

    err = readRequestParser.GetEventPathList(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
        eventPathListParser.GetReader(&eventPathListReader);

        while (CHIP_NO_ERROR == (err = eventPathListReader.Next()))
        {
            VerifyOrExit(TLV::AnonymousTag == eventPathListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

            EventPath::Parser eventPath;

            err = eventPath.Init(eventPathListReader);
            SuccessOrExit(err);
            // TODO: Pass event path to report engine to generate report with interested events
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    MoveToState(HandlerState::Reportable);

    err = InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR ReadHandler::ProcessAttributePathList(AttributePathList::Parser & aAttributePathListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aAttributePathListParser.GetReader(&reader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
        {
            AttributePathParams attributePathParams;
            AttributePath::Parser path;
            NodeId nodeId             = 0;
            EndpointId endpointId     = 0;
            ClusterId clusterId       = 0;
            FieldId fieldId           = 0;
            ClusterInfo * clusterInfo = nullptr;
            err                       = path.Init(reader);
            SuccessOrExit(err);
            err = path.GetNodeId(&nodeId);
            SuccessOrExit(err);
            err = path.GetEndpointId(&endpointId);
            SuccessOrExit(err);
            err = path.GetClusterId(&clusterId);
            SuccessOrExit(err);
            err = path.GetFieldId(&fieldId);
            SuccessOrExit(err);
            attributePathParams.mNodeId     = nodeId;
            attributePathParams.mEndpointId = endpointId;
            attributePathParams.mClusterId  = clusterId;
            attributePathParams.mFieldId    = fieldId;

            for (long i = 0; i < mNumClusterInfos; ++i)
            {
                if (mpClusterInfoList[i].mAttributePathParams.IsSamePath(attributePathParams))
                {
                    clusterInfo = &mpClusterInfoList[i];
                    break;
                }
            }
            if (clusterInfo == nullptr)
            {
                err = InteractionModelEngine::GetInstance()->GetFirstAvailableClusterInfo(clusterInfo);
                SuccessOrExit(err);
                ++mNumClusterInfos;
            }
            clusterInfo->mAttributePathParams = attributePathParams;
            if (nullptr == mpClusterInfoList)
            {
                // this the first cluster instance for this read handler
                // mNumClusterInfoList has already be incremented
                mpClusterInfoList = clusterInfo;
            }
            clusterInfo->SetDirty();
        }
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);
    return err;
}

const char * ReadHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case HandlerState::Uninitialized:
        return "Uninitialized";

    case HandlerState::Initialized:
        return "Initialized";

    case HandlerState::Reportable:
        return "Reportable";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadHandler::MoveToState(const HandlerState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());
}

CHIP_ERROR ReadHandler::GetProcessingClusterInfo(ClusterInfo *& apClusterInfo)
{
    if (mpClusterInfoList == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    apClusterInfo = &mpClusterInfoList[mCurProcessingClusterInfoIdx];
    return CHIP_NO_ERROR;
}

void ReadHandler::ClearClusterInfo()
{
    mpClusterInfoList            = nullptr;
    mCurProcessingClusterInfoIdx = 0;
    mNumClusterInfos             = 0;
}

// Release clusterInfo list for this read handler to pool and shrink the rear of clusterInfo List forward if any
void ReadHandler::ReleaseClusterInfoList()
{
    long numClusterInfosToBeAffected = 0;
    long numberClusterInfosPool = InteractionModelEngine::GetInstance()->GetNumClusterInfos();
    ClusterInfo * clusterInfoPool = InteractionModelEngine::GetInstance()->GetClusterInfoPool();

    if (mNumClusterInfos == 0)
    {
        ChipLogDetail(DataManagement, "No cluster instances allocated");
        return;
    }

    // make sure everything is still sane
    ChipLogIfFalse(mpClusterInfoList >= clusterInfoPool);
    ChipLogIfFalse(mNumClusterInfos <= numberClusterInfosPool);

    // clusterInfoPool + numberClusterInfosPool is a pointer which points to the last+1byte of this array
    // clusterInfoList is a pointer to the first cluster instance to be released
    // the result of subtraction is the number of cluster instances from clusterInfoList to the end of this array
    numClusterInfosToBeAffected = clusterInfoPool + numberClusterInfosPool - mpClusterInfoList;

    // Shrink the clusterInfosInPool by the number of cluster instances.
    InteractionModelEngine::GetInstance()->ShrinkCluster(mNumClusterInfos);

    ChipLogDetail(DataManagement, "Releasing %ld out of %ld", mNumClusterInfos, numClusterInfosToBeAffected);

    if (mNumClusterInfos == numClusterInfosToBeAffected)
    {
        ChipLogDetail(DataManagement, "Releasing the last block of cluster instances");
        return;
    }

    ChipLogDetail(DataManagement, "Moving %d cluster infos forward", numClusterInfosToBeAffected - mNumClusterInfos);

    memmove(mpClusterInfoList, mpClusterInfoList + mNumClusterInfos,
            sizeof(ClusterInfo) * static_cast<size_t>(numClusterInfosToBeAffected - mNumClusterInfos));

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_HANDLER; ++i)
    {
        ReadHandler * handler = InteractionModelEngine::GetInstance()->GetReadHandler(i);

        if ((this != handler) && (handler->mpClusterInfoList > mpClusterInfoList))
        {
            handler->mpClusterInfoList -= mNumClusterInfos;
        }
    }
    ClearClusterInfo();
}

} // namespace app
} // namespace chip
