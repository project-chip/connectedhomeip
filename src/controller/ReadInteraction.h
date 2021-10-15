/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadPrepareParams.h>
#include <controller/TypedReadCallback.h>

namespace chip {
namespace Controller {

/*
 * A typed read attribute function that takes as input a template parameter that encapsulates the type information
 * for a given attribute as well as callbacks for success and failure and either returns a decoded cluster-object representation
 * of the requested attribute through the provided success callback or calls the provided failure callback.
 *
 * The AttributeTypeInfo is generally expected to be a ClusterName::Attributes::AttributeName::TypeInfo struct, but any
 * object that contains type information exposed through a 'using Type = ...' declaration as well as GetClusterId() and
 * GetAttributeId() methods is expected to work.
 *
 */
template <typename AttributeTypeInfo>
CHIP_ERROR ReadAttribute(Messaging::ExchangeManager * aExchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
                         typename TypedReadCallback<AttributeTypeInfo>::OnSuccessCallbackType onSuccessCb,
                         typename TypedReadCallback<AttributeTypeInfo>::OnErrorCallbackType onErrorCb)
{
    app::AttributePathParams attributePath;
    app::ReadPrepareParams readParams(sessionHandle);
    app::ReadClient * readClient         = nullptr;
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    attributePath.mEndpointId = endpointId;
    attributePath.mClusterId  = AttributeTypeInfo::GetClusterId();
    attributePath.mFieldId    = AttributeTypeInfo::GetAttributeId();
    attributePath.mFlags.Set(app::AttributePathParams::Flags::kFieldIdValid);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    auto callback = chip::Platform::MakeUnique<TypedReadCallback<AttributeTypeInfo>>(onSuccessCb, onErrorCb);
    VerifyOrExit(callback != nullptr, err = CHIP_ERROR_NO_MEMORY);

    {
        auto onDone = [rawCallbackPtr = callback.get()]() { chip::Platform::Delete(rawCallbackPtr); };

        callback->SetOnDoneCallback(onDone);

        err = engine->NewReadClient(&readClient, app::ReadClient::InteractionType::Read, 0, callback.get());
        SuccessOrExit(err);

        err = readClient->SendReadRequest(readParams);
        SuccessOrExit(err);

        //
        // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
        // of the read operation to permit us to free up the callback object. So, release ownership of the callback
        // object now to prevent it from being reclaimed at the end of this scoped block.
        //
        callback.release();
        readClient = nullptr;
    }

exit:
    if (readClient && err != CHIP_NO_ERROR)
    {
        readClient->Shutdown();
    }

    return err;
}

} // namespace Controller
} // namespace chip
