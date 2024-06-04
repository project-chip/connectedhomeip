/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/ReadHandler.h>

namespace chip {
namespace Test {

using namespace app;
/**
 * @brief Class acts as an accessor to private methods of the ReadHandler class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */
class ReadHandlerTestAccess
{

public:
    ReadHandlerTestAccess(ReadHandler * apReadHandler) : mpReadHandler(apReadHandler) {}

    Messaging::ExchangeHolder & GetExchangeCtx() { return mpReadHandler->mExchangeCtx; }
    ReadHandler * GetReadHandler() { return mpReadHandler; }
    SessionHolder & GetSessionHandle() { return mpReadHandler->mSessionHandle; }
    Transport::SecureSession * GetSession() { return mpReadHandler->GetSession(); }
    ReadHandler::Observer * GetObserver() { return mpReadHandler->mObserver; }

    bool IsDirty() const { return mpReadHandler->IsDirty(); }
    bool ShouldStartReporting() const { return mpReadHandler->ShouldStartReporting(); }

    void ForceDirtyState() { mpReadHandler->ForceDirtyState(); }
    void ClearForceDirtyFlag() { mpReadHandler->ClearForceDirtyFlag(); }
    void MoveHandlerToIdleState() { mpReadHandler->MoveToState(ReadHandler::HandlerState::Idle); }

    void SetStateFlagToActiveSubscription(bool aValue)
    {
        mpReadHandler->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, aValue);
    }

    void OnInitialRequest(System::PacketBufferHandle && aPayload)
    {
        if (mpReadHandler != nullptr)
        {
            mpReadHandler->OnInitialRequest(std::move(aPayload));
        }
    }

    CHIP_ERROR SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks)
    {
        return mpReadHandler->SendReportData(std::move(aPayload), aMoreChunks);
    }

    CHIP_ERROR ProcessSubscribeRequest(System::PacketBufferHandle && aPayload)
    {
        return mpReadHandler->ProcessSubscribeRequest(std::move(aPayload));
    }

    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload)
    {
        return mpReadHandler->ProcessReadRequest(std::move(aPayload));
    }

private:
    ReadHandler * mpReadHandler = nullptr;
};

} // namespace Test
} // namespace chip
