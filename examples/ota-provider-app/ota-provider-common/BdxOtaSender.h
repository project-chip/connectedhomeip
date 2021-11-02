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

// Callback to handle block query
typedef void (*OnBdxBlockQuery)(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof,
                                uint32_t offset);
typedef void (*OnBdxTransferComplete)(void * context);
typedef void (*OnBdxTransferFailed)(void * context);

struct BdxOtaSenderCallbacks
{
    chip::Callback::Callback<OnBdxBlockQuery> * onBlockQuery             = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * onTransferComplete = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * onTransferFailed     = nullptr;
};

class BdxOtaSender : public chip::bdx::Responder
{
public:
    void SetCallbacks(BdxOtaSenderCallbacks callbacks);

private:
    // Inherited from bdx::TransferFacilitator
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

    uint32_t mNumBytesSent = 0;

    chip::Callback::Callback<OnBdxBlockQuery> * mOnBlockQueryCallback             = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * mOnTransferCompleteCallback = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * mOnTransferFailedCallback     = nullptr;
};
