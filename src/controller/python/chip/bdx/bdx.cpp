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
    
} // namespace python
} // namespace chip

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
    // TODO: Call BdxTransferManager::ExpectATransfer.
}

PyChipError pychip_Bdx_StopExpectingBdxTransfer()
{
    // TODO: Call BdxTransferManager::StopExpectingATransfer.
}

PyChipError pychip_Bdx_AcceptSendTransfer(void * transfer, PyObject dataReceivedContext, PyObject transferCompletedContext)
{
    // TODO: Pass transferCompletedContext to transfer so that the system will call gOnTransferCompletedCallback with
    //       transferCompletedContext eventually.
    // TODO: Call transfer->AcceptSend with a data callback to call gOnDataReceivedCallback with dataReceivedContext.
}

PyChipError pychip_Bdx_AcceptReceiveTransfer(void * transfer, const uint8_t * dataBuffer, size_t dataLength,
                                             void * transferCompletedContext)
{
    // TODO: Pass transferCompletedContext to transfer so that the system will call gOnTransferCompletedCallback with
    //       transferCompletedContext eventually.
    // TODO: Call transfer->AcceptReceive with the data.
}

PyChipError pychip_Bdx_RejectTransfer(void * transfer)
{
    // TODO: Pass transferCompletedContext to transfer so that the system will call gOnTransferCompletedCallback with
    //       transferCompletedContext eventually.
    // TODO: Call transfer->Reject.
}

}
