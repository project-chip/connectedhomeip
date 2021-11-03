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

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#pragma once

enum BdxDownloaderErrorTypes
{
    kErrorBdxDownloaderNoError = 0,
    kErrorBdxDownloaderStatusReceived,
    kErrorBdxDownloaderInternal,
    kErrorBdxDownloaderTimeOut,
};

/**
 * @brief
 *   This callback is called when bdx transfer receives Block or BlockEOF message
 *
 * @param[in] context   User context
 * @param[in] blockData BlockData structure which contains pointer to data, length of data and IsEof flag
 */
typedef void (*OnBdxBlockReceived)(void * context, const chip::bdx::TransferSession::BlockData & blockdata);

/**
 * @brief
 *   This callback is called after BlockEOF message is processed
 *
 * @param[in] context User context
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
typedef void (*OnBdxTransferFailed)(void * context, BdxDownloaderErrorTypes status);

// TODO: With this approach we might end up adding callback for every bdx event/message.
//       Can be refactored into a single callback with events
struct BdxDownloaderCallbacks
{
    chip::Callback::Callback<OnBdxBlockReceived> * onBlockReceived       = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * onTransferComplete = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * onTransferFailed     = nullptr;
};

class BdxDownloader : public chip::bdx::Initiator
{
public:
    void SetInitialExchange(chip::Messaging::ExchangeContext * ec);

    void SetCallbacks(BdxDownloaderCallbacks callbacks);

private:
    // inherited from bdx::Endpoint
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event);

    bool mIsTransferComplete = false;

    chip::Callback::Callback<OnBdxBlockReceived> * mOnBlockReceivedCallback       = nullptr;
    chip::Callback::Callback<OnBdxTransferComplete> * mOnTransferCompleteCallback = nullptr;
    chip::Callback::Callback<OnBdxTransferFailed> * mOnTransferFailedCallback     = nullptr;
};
