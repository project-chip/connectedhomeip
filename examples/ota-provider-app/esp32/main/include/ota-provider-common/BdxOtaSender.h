/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#pragma once

enum BdxSenderErrorTypes
{
    kErrorBdxSenderNoError = 0,
    kErrorBdxSenderStatusReceived,
    kErrorBdxSenderInternal,
    kErrorBdxSenderTimeOut,
};

/**
 * @brief
 *   This callback is called when bdx session receives a BlockQuery message.
 *
 * @param[in]  context  User context
 * @param[out] blockBuf Preallocated PacketBuffer, implementor should fill the block buf with the data
 * @param[out] size     Size of data
 * @param[out] isEof    Flag which tells whether it is final block or not
 * @param[in]  offset   Offset to read data from
 *
 * @return CHIP_NO_ERROR on success; return appropriate error code otherwise
 */
typedef CHIP_ERROR (*OnBdxBlockQuery)(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof,
                                      uint32_t offset);

/**
 * @brief
 *   This callback is called when bdx session receives BlockAckEOF message
 *
 * @param[in] context User contex
 */
typedef void (*OnBdxTransferComplete)(void * context);

/**
 * @brief
 *   This callback is called when bdx transfer receives StatusReport messages,
 *   if there is any internal error, or transfer timed out
 *
 * @param[in] context User context
 * @param[in] status Error code
 */
typedef void (*OnBdxTransferFailed)(void * context, BdxSenderErrorTypes status);

struct BdxOtaSenderCallbacks
{
    chip::Callback::Callback<OnBdxBlockQuery> * onBlockQuery             = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * onTransferComplete = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * onTransferFailed     = nullptr;
};

class BdxOtaSender : public chip::bdx::Responder
{
public:
    BdxOtaSender() { memset(mFileDesignator, 0, sizeof(mFileDesignator)); }

    // Initializes BDX transfer-related metadata. Should always be called first.
    CHIP_ERROR InitializeTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    void SetCallbacks(BdxOtaSenderCallbacks callbacks);

    /**
     * @brief
     *   Get negotiated bdx tranfer block size
     *
     * @return Negotiated bdx tranfer block size
     */
    uint16_t GetTransferBlockSize(void);

    /**
     * @brief
     *   Get the predetermined definite length for the transfer
     *
     * @return Predetermined definite length for the transfer
     */
    uint64_t GetTransferLength(void);

    /**
     * @brief
     *  Get the file designator for the transfer
     *
     * @return File designator for the transfer
     */
    const char * GetFileDesignator() const { return mFileDesignator; }

private:
    // Inherited from bdx::TransferFacilitator
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

    uint32_t mNumBytesSent = 0;

    bool mInitialized = false;

    chip::Optional<chip::FabricIndex> mFabricIndex;

    chip::Optional<chip::NodeId> mNodeId;

    chip::Callback::Callback<OnBdxBlockQuery> * mOnBlockQueryCallback             = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * mOnTransferCompleteCallback = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * mOnTransferFailedCallback     = nullptr;

    // Maximum file designator length
    static constexpr uint8_t kMaxFDLen = 30;
    // Null-terminated string representing file designator
    char mFileDesignator[kMaxFDLen];
};
