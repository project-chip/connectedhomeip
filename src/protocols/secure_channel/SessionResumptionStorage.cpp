/*
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#include <protocols/secure_channel/SessionResumptionStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>

namespace chip {

CHIP_ERROR SessionResumptionStorage::FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                                        Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)
{
    ReturnErrorOnFailure(LoadState(node, resumptionId, sharedSecret, peerCATs));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                                        Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)
{
    ReturnErrorOnFailure(FindNodeByResumptionId(resumptionId, node));
    ResumptionIdStorage tmpResumptionId;
    ReturnErrorOnFailure(FindByScopedNodeId(node, tmpResumptionId, sharedSecret, peerCATs));
    VerifyOrReturnError(std::equal(tmpResumptionId.begin(), tmpResumptionId.end(), resumptionId.begin(), resumptionId.end()),
                        CHIP_ERROR_KEY_NOT_FOUND);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::FindNodeByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node)
{
    ReturnErrorOnFailure(LoadLink(resumptionId, node));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::Save(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                                          const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs)
{
    SessionIndex index;
    ReturnErrorOnFailure(LoadIndex(index));

    if (index.mSize == CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE)
    {
        // TODO: implement LRU for resumption
        ReturnErrorOnFailure(Delete(index.mNodes[0]));
        ReturnErrorOnFailure(LoadIndex(index));
    }

    ReturnErrorOnFailure(SaveState(node, resumptionId, sharedSecret, peerCATs));
    ReturnErrorOnFailure(SaveLink(resumptionId, node));

    index.mNodes[index.mSize++] = node;
    ReturnErrorOnFailure(SaveIndex(index));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::Delete(const ScopedNodeId & node)
{
    SessionIndex index;
    ReturnErrorOnFailure(LoadIndex(index));

    ResumptionIdStorage resumptionId;
    Crypto::P256ECDHDerivedSecret sharedSecret;
    CATValues peerCATs;
    CHIP_ERROR err = LoadState(node, resumptionId, sharedSecret, peerCATs);
    if (err == CHIP_NO_ERROR)
    {
        err = DeleteLink(resumptionId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel,
                         "Unable to delete session resumption link for node " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(node.GetNodeId()), err.Format());
        }
    }
    else
    {
        ChipLogError(SecureChannel,
                     "Unable to load session resumption state during session deletion for node " ChipLogFormatX64
                     ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(node.GetNodeId()), err.Format());
    }

    err = DeleteState(node);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Unable to delete session resumption state for node " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(node.GetNodeId()), err.Format());
    }

    bool found = false;
    for (size_t i = 0; i < index.mSize; ++i)
    {
        if (found)
        {
            // index.mSize was decreased by 1 when found was set to true.
            // So the (i+1)th element isn't out of bounds.
            index.mNodes[i] = index.mNodes[i + 1];
        }
        else
        {
            if (index.mNodes[i] == node)
            {
                found = true;
                if (i + 1 < index.mSize)
                {
                    index.mNodes[i] = index.mNodes[i + 1];
                }
                index.mSize -= 1;
            }
        }
    }

    if (found)
    {
        err = SaveIndex(index);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel, "Unable to save session resumption index: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else
    {
        ChipLogError(SecureChannel,
                     "Unable to find session resumption state for node in index" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(node.GetNodeId()), err.Format());
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
