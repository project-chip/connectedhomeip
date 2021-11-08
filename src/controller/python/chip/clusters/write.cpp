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

#include <cstdarg>
#include <memory>
#include <type_traits>

#include <app/WriteClient.h>
#include <controller/CHIPDevice.h>
#include <lib/support/CodeUtils.h>

#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

using PyObject = void *;

extern "C" {
// Encodes n attribute write requests, follows 3 * n arguments, in the (AttributeWritePath*=void *, uint8_t*, size_t) order.
chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, Controller::Device * device, size_t n, ...);
}

namespace chip {
namespace python {

struct __attribute__((packed)) AttributeWritePath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::AttributeId attributeId;
};

using OnWriteResponseCallback = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                         chip::AttributeId attributeId,
                                         std::underlying_type_t<Protocols::InteractionModel::Status> imstatus);
using OnWriteErrorCallback    = void (*)(PyObject appContext, uint32_t chiperror);
using OnWriteDoneCallback     = void (*)(PyObject appContext);

OnWriteResponseCallback gOnWriteResponseCallback = nullptr;
OnWriteErrorCallback gOnWriteErrorCallback       = nullptr;
OnWriteDoneCallback gOnWriteDoneCallback         = nullptr;

class WriteClientCallback : public WriteClient::Callback
{
public:
    WriteClientCallback(PyObject appContext) : mAppContext(appContext) {}

    void OnResponse(const WriteClient * apWriteClient, const ConcreteAttributePath & aPath, app::StatusIB aStatus) override
    {
        gOnWriteResponseCallback(mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                                 to_underlying(aStatus.mStatus));
    }

    void OnError(const WriteClient * apWriteClient, CHIP_ERROR aProtocolError) override
    {
        gOnWriteErrorCallback(mAppContext, aProtocolError.AsInteger());
    }

    void OnDone(WriteClient * apCommandSender) override
    {
        gOnWriteDoneCallback(mAppContext);
        // delete apCommandSender;
        delete this;
    };

private:
    PyObject mAppContext = nullptr;
};

} // namespace python
} // namespace chip

using namespace chip::python;

extern "C" {
void pychip_WriteClient_InitCallbacks(OnWriteResponseCallback onWriteResponseCallback, OnWriteErrorCallback onWriteErrorCallback,
                                      OnWriteDoneCallback onWriteDoneCallback)
{
    gOnWriteResponseCallback = onWriteResponseCallback;
    gOnWriteErrorCallback    = onWriteErrorCallback;
    gOnWriteDoneCallback     = onWriteDoneCallback;
}

chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, Controller::Device * device, size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<WriteClientCallback> callback = std::make_unique<WriteClientCallback>(appContext);
    app::WriteClientHandle client;

    va_list args;
    va_start(args, n);

    SuccessOrExit(err = app::InteractionModelEngine::GetInstance()->NewWriteClient(client, callback.get()));

    {
        for (size_t i = 0; i < n; i++)
        {
            void * path = va_arg(args, void *);
            void * tlv  = va_arg(args, void *);
            int length  = va_arg(args, int);

            AttributeWritePath pathObj;
            memcpy(&pathObj, path, sizeof(AttributeWritePath));
            uint8_t * tlvBuffer = reinterpret_cast<uint8_t *>(tlv);

            TLV::TLVWriter * writer;
            TLV::TLVReader reader;

            SuccessOrExit(err = client->PrepareAttribute(
                              chip::app::AttributePathParams(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId)));
            VerifyOrExit((writer = client->GetAttributeDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

            reader.Init(tlvBuffer, static_cast<uint32_t>(length));
            reader.Next();
            SuccessOrExit(err = writer->CopyElement(
                              chip::TLV::ContextTag(chip::to_underlying(chip::app::AttributeDataIB::Tag::kData)), reader));

            SuccessOrExit(err = client->FinishAttribute());
        }
    }

    SuccessOrExit(err = device->SendWriteAttributeRequest(std::move(client), nullptr, nullptr));

    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}
}
