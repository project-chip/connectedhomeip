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

#include "../linux/OTAImageProcessorLinux.h"

#pragma once

class BdxDownloader : public chip::bdx::Initiator
{
public:
    BdxDownloader() { mImageProcessor = chip::Platform::New<chip::OTAImageProcessorLinux>(); }
    void SetInitialExchange(chip::Messaging::ExchangeContext * ec);

private:
    // inherited from bdx::Endpoint
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event);

    chip::OTAImageProcessorInterface * mImageProcessor;
    bool mIsTransferComplete = false;
};
