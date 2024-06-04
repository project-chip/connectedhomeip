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

#include <protocols/bdx/BdxMessages.h>

#include <controller/python/chip/bdx/bdx-transfer.h>
#include <controller/python/chip/bdx/bdx-transfer-manager.h>
#include <controller/python/chip/bdx/bdx-transfer-server.h>
#include <controller/python/chip/native/PyChipError.h>

using PyObject = void *;

namespace chip {
namespace python {

using OnTransferObtainedCallback = void (*)(PyObject context, PyChipError result, void * bdxTransfer,
                                            bdx::TransferControlFlags transferControlFlags, uint16_t maxBlockSize,
                                            uint64_t startOffset, uint64_t length, const uint8_t * fileDesignator,
                                            uint16_t fileDesignatorLength, const uint8_t * metadata, size_t metadataLength);
using OnDataReceivedCallback = void (*)(PyObject context, const uint8_t * dataBuffer, size_t bufferLength);
using OnTransferCompletedCallback = void (*)(PyObject context, PyChipError result);

OnTransferObtainedCallback gOnTransferObtainedCallback   = nullptr;
OnDataReceivedCallback gOnDataReceivedCallback           = nullptr;
OnTransferCompletedCallback gOnTransferCompletedCallback = nullptr;

struct TransferData
{
    PyObject OnTransferObtainedContext = nullptr;
    PyObject OnDataReceivedContext = nullptr;
    PyObject OnTransferCompletedContext = nullptr;
};

} // namespace python

namespace bdx {

class BdxTransferDelegate : public BdxTransfer::Delegate
{
public:
    ~BdxTransferDelegate() override = default;

    virtual void InitMessageReceived(BdxTransfer * transfer, TransferSession::TransferInitData init_data)
    {
        if (gOnTransferObtainedCallback)
        {
            // TODO: Get the transfer data from transfer.
            python::TransferData * transferData = nullptr;
            PyChipError result;
            gOnTransferObtainedCallback(transferData->OnTransferObtainedContext, result, transfer, init_data.TransferCtlFlags,
                                        init_data.MaxBlockSize, init_data.StartOffset, init_data.Length, init_data.FileDesignator,
                                        init_data.FileDesLength, init_data.Metadata, init_data.MetadataLength);
        }
    }

    virtual void DataReceived(BdxTransfer * transfer, const ByteSpan & block)
    {
        if (gOnDataReceivedCallback)
        {
            // TODO: Get the transfer data from transfer.
            python::TransferData * transferData = nullptr;
            gOnDataReceivedCallback(transferData->OnDataReceivedContext, block.data(), block.size());
        }
    }

    virtual void TransferCompleted(BdxTransfer * transfer, CHIP_ERROR result)
    {
        if (gOnTransferCompletedCallback)
        {
            // TODO: Get the transfer data from transfer.
            python::TransferData * transferData = nullptr;
            gOnTransferCompletedCallback(transferData->OnTransferCompletedContext, ToPyChipError(result));
        }
    }
};

} // namespace bdx
} // namespace chip

namespace {

chip::bdx::BdxTransferDelegate gBdxTransferDelegate;
chip::bdx::BdxTransferManager gBdxTransferManager(&gBdxTransferDelegate);
chip::bdx::BdxTransferServer gBdxTransferServer(gBdxTransferManager);

} // namespace

using namespace chip::python;

extern "C" {

void pychip_Bdx_InitCallbacks(OnTransferObtainedCallback onTransferObtainedCallback,
                              OnDataReceivedCallback onDataReceivedCallback,
                              OnTransferCompletedCallback onTransferCompletedCallback)
{
    OnTransferObtainedCallback gOnTransferObtainedCallback   = onTransferObtainedCallback;
    OnDataReceivedCallback gOnDataReceivedCallback           = onDataReceivedCallback;
    OnTransferCompletedCallback gOnTransferCompletedCallback = onTransferCompletedCallback;
}

PyChipError pychip_Bdx_ExpectBdxTransfer()
{
    gBdxTransferManager.ExpectATransfer();
}

PyChipError pychip_Bdx_StopExpectingBdxTransfer()
{
    gBdxTransferManager.StopExpectingATransfer();
}

PyChipError pychip_Bdx_AcceptSendTransfer(chip::bdx::BdxTransfer * transfer, PyObject dataReceivedContext,
                                          PyObject transferCompletedContext)
{
    // TODO: Get the transfer data from transfer.
    TransferData * transferData = nullptr;
    transferData->OnDataReceivedContext = dataReceivedContext;
    transferData->OnTransferCompletedContext = transferCompletedContext;
    transfer->AcceptSend();
}

PyChipError pychip_Bdx_AcceptReceiveTransfer(chip::bdx::BdxTransfer * transfer, const uint8_t * dataBuffer, size_t dataLength,
                                             PyObject transferCompletedContext)
{
    // TODO: Get the transfer data from transfer.
    TransferData * transferData = nullptr;
    transferData->OnTransferCompletedContext = transferCompletedContext;
    chip::ByteSpan data(dataBuffer, dataLength);
    transfer->AcceptReceive(data);
}

PyChipError pychip_Bdx_RejectTransfer(chip::bdx::BdxTransfer * transfer)
{
    transfer->Reject();
}

}
