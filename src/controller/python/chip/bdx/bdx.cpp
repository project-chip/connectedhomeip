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

#include <vector>

#include <protocols/bdx/BdxMessages.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/python/chip/bdx/bdx-transfer.h>
#include <controller/python/chip/bdx/test-bdx-transfer-server.h>
#include <controller/python/chip/native/PyChipError.h>

// The BDX transfer system is split into:
// * BdxTransfer: A transfer object that contains the information about a transfer and is an ExchangeDelegate.
//   It owns the data for a transfer, either copying what was sent from Python or requiring the Python side to
//   copy it during a callback.
// * TransferMap: A map that associates the BdxTransfer object with its Python context using TransferInfo objects.
//   It owns the TransferInfo objects but doesn't own the BdxTransfer objects or the Python context objects.
// * TransferDelegate: A delegate that calls back into Python when certain events happen in C++. It uses the
//   TransferMap but doesn't own it.
// TestBdxTransferServer: A server that listens for incoming BDX messages, creates BdxTransfer objects, and
//   informs the delegate when certain events happen. It owns the BdxTransfer objects but not the delegate. A
//   BdxTransfer object is created when a BDX message is received and destroyed when the transfer completes or
//   fails.
// The TransferMap, TransferDelegate, and TestBdxTransferServer instances are all owned by this file.

using PyObject = void *;

namespace chip {
namespace python {

// The Python callbacks to call when certain events happen.
using OnTransferObtainedCallback = void (*)(PyObject context, void * bdxTransfer, bdx::TransferControlFlags transferControlFlags,
                                            uint16_t maxBlockSize, uint64_t startOffset, uint64_t length,
                                            const uint8_t * fileDesignator, uint16_t fileDesignatorLength, const uint8_t * metadata,
                                            size_t metadataLength);
using OnFailedToObtainTransferCallback = void (*)(PyObject context, PyChipError result);
using OnDataReceivedCallback           = void (*)(PyObject context, const uint8_t * dataBuffer, size_t bufferLength);
using OnTransferCompletedCallback      = void (*)(PyObject context, PyChipError result);

// The callback methods provided by python.
OnTransferObtainedCallback gOnTransferObtainedCallback             = nullptr;
OnFailedToObtainTransferCallback gOnFailedToObtainTransferCallback = nullptr;
OnDataReceivedCallback gOnDataReceivedCallback                     = nullptr;
OnTransferCompletedCallback gOnTransferCompletedCallback           = nullptr;

// The information for a single transfer.
struct TransferInfo
{
    // The transfer object. Owned by the transfer server.
    bdx::BdxTransfer * Transfer = nullptr;
    // The contexts for different python callbacks. Owned by the python side.
    PyObject OnTransferObtainedContext  = nullptr;
    PyObject OnDataReceivedContext      = nullptr;
    PyObject OnTransferCompletedContext = nullptr;

    bool operator==(const TransferInfo & other) const { return Transfer == other.Transfer; }
};

// The set of transfers.
class TransferMap
{
public:
    // Returns the transfer data associated with the given transfer.
    TransferInfo * TransferInfoForTransfer(bdx::BdxTransfer * transfer)
    {
        std::vector<TransferInfo>::iterator result = std::find_if(
            mTransfers.begin(), mTransfers.end(), [transfer](const TransferInfo & data) { return data.Transfer == transfer; });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    // Returns the transfer data that has the given context when a transfer is obtained.
    TransferInfo * TransferInfoForTransferObtainedContext(PyObject transferObtainedContext)
    {
        std::vector<TransferInfo>::iterator result =
            std::find_if(mTransfers.begin(), mTransfers.end(), [transferObtainedContext](const TransferInfo & data) {
                return data.OnTransferObtainedContext == transferObtainedContext;
            });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    // This returns the next transfer data that has no associated BdxTransfer.
    TransferInfo * NextUnassociatedTransferInfo()
    {
        std::vector<TransferInfo>::iterator result =
            std::find_if(mTransfers.begin(), mTransfers.end(), [](const TransferInfo & data) { return data.Transfer == nullptr; });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    // Creates a new transfer data.
    TransferInfo * CreateUnassociatedTransferInfo() { return &mTransfers.emplace_back(); }

    void RemoveTransferInfo(TransferInfo * transferInfo)
    {
        std::vector<TransferInfo>::iterator result = std::find(mTransfers.begin(), mTransfers.end(), *transferInfo);
        VerifyOrReturn(result != mTransfers.end());
        mTransfers.erase(result);
    }

private:
    std::vector<TransferInfo> mTransfers;
};

// A method to release a transfer.
void ReleaseTransfer(System::Layer * systemLayer, bdx::BdxTransfer * transfer);

// A delegate to forward events from a transfer to the appropriate Python callback and context.
class TransferDelegate : public bdx::BdxTransfer::Delegate
{
public:
    TransferDelegate(TransferMap * transfers) : mTransfers(transfers) {}
    ~TransferDelegate() override = default;

    void Init(System::Layer * systemLayer) { mSystemLayer = systemLayer; }

    void InitMessageReceived(bdx::BdxTransfer * transfer, bdx::TransferSession::TransferInitData init_data) override
    {
        TransferInfo * transferInfo = mTransfers->NextUnassociatedTransferInfo();
        if (gOnTransferObtainedCallback && transferInfo)
        {
            transferInfo->Transfer = transfer;
            gOnTransferObtainedCallback(transferInfo->OnTransferObtainedContext, transfer, init_data.TransferCtlFlags,
                                        init_data.MaxBlockSize, init_data.StartOffset, init_data.Length, init_data.FileDesignator,
                                        init_data.FileDesLength, init_data.Metadata, init_data.MetadataLength);
        }
    }

    void DataReceived(bdx::BdxTransfer * transfer, const ByteSpan & block) override
    {
        TransferInfo * transferInfo = mTransfers->TransferInfoForTransfer(transfer);
        if (gOnDataReceivedCallback && transferInfo)
        {
            gOnDataReceivedCallback(transferInfo->OnDataReceivedContext, block.data(), block.size());
        }
    }

    void TransferCompleted(bdx::BdxTransfer * transfer, CHIP_ERROR result) override
    {
        TransferInfo * transferInfo = mTransfers->TransferInfoForTransfer(transfer);
        if (!transferInfo && result != CHIP_NO_ERROR)
        {
            // The transfer failed during initialisation.
            transferInfo = mTransfers->NextUnassociatedTransferInfo();
            if (gOnFailedToObtainTransferCallback && transferInfo)
            {
                gOnFailedToObtainTransferCallback(transferInfo->OnTransferObtainedContext, ToPyChipError(result));
            }
        }
        else if (gOnTransferCompletedCallback && transferInfo)
        {
            gOnTransferCompletedCallback(transferInfo->OnTransferCompletedContext, ToPyChipError(result));
            mTransfers->RemoveTransferInfo(transferInfo);
        }
        ReleaseTransfer(mSystemLayer, transfer);
    }

private:
    TransferMap * mTransfers     = nullptr;
    System::Layer * mSystemLayer = nullptr;
};

TransferMap gTransfers;
TransferDelegate gBdxTransferDelegate(&gTransfers);
bdx::TestBdxTransferServer gBdxTransferServer(&gBdxTransferDelegate);

void ReleaseTransfer(System::Layer * systemLayer, bdx::BdxTransfer * transfer)
{
    systemLayer->ScheduleWork(
        [](auto * theSystemLayer, auto * appState) -> void {
            auto * theTransfer = static_cast<bdx::BdxTransfer *>(appState);
            gBdxTransferServer.Release(theTransfer);
        },
        transfer);
}

} // namespace python
} // namespace chip

using namespace chip::python;

// These methods are expected to be called from Python.
extern "C" {

// Initialises the BDX system.
void pychip_Bdx_InitCallbacks(OnTransferObtainedCallback onTransferObtainedCallback,
                              OnFailedToObtainTransferCallback onFailedToObtainTransferCallback,
                              OnDataReceivedCallback onDataReceivedCallback,
                              OnTransferCompletedCallback onTransferCompletedCallback)
{
    gOnTransferObtainedCallback                         = onTransferObtainedCallback;
    gOnFailedToObtainTransferCallback                   = onFailedToObtainTransferCallback;
    gOnDataReceivedCallback                             = onDataReceivedCallback;
    gOnTransferCompletedCallback                        = onTransferCompletedCallback;
    chip::Controller::DeviceControllerFactory & factory = chip::Controller::DeviceControllerFactory::GetInstance();
    chip::System::Layer * systemLayer                   = factory.GetSystemState()->SystemLayer();
    gBdxTransferDelegate.Init(systemLayer);
    gBdxTransferServer.Init(systemLayer, factory.GetSystemState()->ExchangeMgr());
}

// Prepares the BDX system to expect a new transfer.
PyChipError pychip_Bdx_ExpectBdxTransfer(PyObject transferObtainedContext)
{
    TransferInfo * transferInfo = gTransfers.CreateUnassociatedTransferInfo();
    VerifyOrReturnValue(transferInfo != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    transferInfo->OnTransferObtainedContext = transferObtainedContext;
    gBdxTransferServer.ExpectATransfer();
    return ToPyChipError(CHIP_NO_ERROR);
}

// Stops expecting a transfer.
PyChipError pychip_Bdx_StopExpectingBdxTransfer(PyObject transferObtainedContext)
{
    TransferInfo * transferInfo = gTransfers.TransferInfoForTransferObtainedContext(transferObtainedContext);
    VerifyOrReturnValue(transferInfo != nullptr, ToPyChipError(CHIP_ERROR_NOT_FOUND));
    gBdxTransferServer.StopExpectingATransfer();
    gTransfers.RemoveTransferInfo(transferInfo);
    return ToPyChipError(CHIP_NO_ERROR);
}

// Accepts a transfer with the intent to receive data from the other device.
PyChipError pychip_Bdx_AcceptTransferAndReceiveData(chip::bdx::BdxTransfer * transfer, PyObject dataReceivedContext,
                                                    PyObject transferCompletedContext)
{
    TransferInfo * transferInfo              = gTransfers.TransferInfoForTransfer(transfer);
    transferInfo->OnDataReceivedContext      = dataReceivedContext;
    transferInfo->OnTransferCompletedContext = transferCompletedContext;
    return ToPyChipError(transfer->AcceptAndReceiveData());
}

// Accepts a transfer with the intent to send data to the other device.
PyChipError pychip_Bdx_AcceptTransferAndSendData(chip::bdx::BdxTransfer * transfer, const uint8_t * dataBuffer, size_t dataLength,
                                                 PyObject transferCompletedContext)
{
    TransferInfo * transferInfo              = gTransfers.TransferInfoForTransfer(transfer);
    transferInfo->OnTransferCompletedContext = transferCompletedContext;
    chip::ByteSpan data(dataBuffer, dataLength);
    return ToPyChipError(transfer->AcceptAndSendData(data));
}

// Rejects a transfer.
PyChipError pychip_Bdx_RejectTransfer(chip::bdx::BdxTransfer * transfer)
{
    return ToPyChipError(transfer->Reject());
}
}
