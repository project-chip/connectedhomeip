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
    bdx::BdxTransfer * Transfer = nullptr;
    PyObject OnTransferObtainedContext = nullptr;
    PyObject OnDataReceivedContext = nullptr;
    PyObject OnTransferCompletedContext = nullptr;

    bool operator==(const TransferData& other) const {
        return Transfer == other.Transfer;
    }
};

class TransferMap
{
public:
    // This returns the transfer data associated with the given transfer.
    TransferData * TransferDataForTransfer(bdx::BdxTransfer * transfer)
    {
        std::vector<TransferData>::iterator result = std::find_if(mTransfers.begin(), mTransfers.end(),
                                                                  [transfer](const TransferData& data) {
                                                                      return data.Transfer == transfer;
                                                                  });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    TransferData * TransferDataForTransferObtainedContext(PyObject transferObtainedContext)
    {
        std::vector<TransferData>::iterator result = std::find_if(mTransfers.begin(), mTransfers.end(),
                                                                  [transferObtainedContext](const TransferData& data) {
                                                                      return data.OnTransferObtainedContext == transferObtainedContext;
                                                                  });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    // This returns the next transfer data that has no associated BdxTransfer.
    TransferData * NextUnassociatedTransferData()
    {
        std::vector<TransferData>::iterator result = std::find_if(mTransfers.begin(), mTransfers.end(),
                                                                  [](const TransferData& data) {
                                                                      return data.Transfer == nullptr;
                                                                  });
        VerifyOrReturnValue(result != mTransfers.end(), nullptr);
        return &*result;
    }

    TransferData * CreateUnassociatedTransferData()
    {
        return &mTransfers.emplace_back();
    }

    void RemoveTransferData(TransferData * transferData)
    {
        std::vector<TransferData>::iterator result = std::find(mTransfers.begin(), mTransfers.end(), *transferData);
        VerifyOrReturn(result != mTransfers.end());
        mTransfers.erase(result);
    }

private:
    std::vector<TransferData> mTransfers;
};

class TransferDelegate : public bdx::BdxTransfer::Delegate
{
public:
    TransferDelegate(TransferMap * transfers) : mTransfers(transfers) {}
    ~TransferDelegate() override = default;

    virtual void InitMessageReceived(bdx::BdxTransfer * transfer, bdx::TransferSession::TransferInitData init_data)
    {
        TransferData * transferData = mTransfers->NextUnassociatedTransferData();
        if (gOnTransferObtainedCallback && transferData)
        {
            transferData->Transfer = transfer;
            gOnTransferObtainedCallback(transferData->OnTransferObtainedContext, ToPyChipError(CHIP_NO_ERROR), transfer,
                                        init_data.TransferCtlFlags, init_data.MaxBlockSize, init_data.StartOffset,
                                        init_data.Length, init_data.FileDesignator, init_data.FileDesLength, init_data.Metadata,
                                        init_data.MetadataLength);
        }
    }

    virtual void DataReceived(bdx::BdxTransfer * transfer, const ByteSpan & block)
    {
        TransferData * transferData = mTransfers->TransferDataForTransfer(transfer);
        if (gOnDataReceivedCallback && transferData)
        {
            gOnDataReceivedCallback(transferData->OnDataReceivedContext, block.data(), block.size());
        }
    }

    virtual void TransferCompleted(bdx::BdxTransfer * transfer, CHIP_ERROR result)
    {
        TransferData * transferData = mTransfers->TransferDataForTransfer(transfer);
        if (!transferData && result != CHIP_NO_ERROR)
        {
            // The transfer failed during initialisation.
            transferData = mTransfers->NextUnassociatedTransferData();
            if (gOnTransferObtainedCallback && transferData)
            {
                gOnTransferObtainedCallback(transferData->OnTransferObtainedContext, ToPyChipError(result), nullptr,
                                            static_cast<bdx::TransferControlFlags>(0), 0, 0, 0, nullptr, 0, nullptr, 0);
            }
        }
        else if (gOnTransferCompletedCallback && transferData)
        {
            gOnTransferCompletedCallback(transferData->OnTransferCompletedContext, ToPyChipError(result));
            mTransfers->RemoveTransferData(transferData);
        }
    }

private:
    TransferMap * mTransfers = nullptr;
};

TransferMap gTransfers;
TransferDelegate gBdxTransferDelegate(&gTransfers);
bdx::BdxTransferManager gBdxTransferManager(&gBdxTransferDelegate);
bdx::BdxTransferServer gBdxTransferServer(gBdxTransferManager);

} // namespace python
} // namespace chip

using namespace chip::python;

extern "C" {

void pychip_Bdx_InitCallbacks(OnTransferObtainedCallback onTransferObtainedCallback,
                              OnDataReceivedCallback onDataReceivedCallback,
                              OnTransferCompletedCallback onTransferCompletedCallback)
{
    gOnTransferObtainedCallback  = onTransferObtainedCallback;
    gOnDataReceivedCallback      = onDataReceivedCallback;
    gOnTransferCompletedCallback = onTransferCompletedCallback;
    // TODO: Move this into its own method maybe.
    chip::Controller::DeviceControllerFactory & factory = chip::Controller::DeviceControllerFactory::GetInstance();
    gBdxTransferManager.Init(factory.GetSystemState()->SystemLayer());
    gBdxTransferServer.Init(factory.GetSystemState()->ExchangeMgr());
}

PyChipError pychip_Bdx_ExpectBdxTransfer(PyObject transferObtainedContext)
{
    TransferData * transferData = gTransfers.CreateUnassociatedTransferData();
    VerifyOrReturnValue(transferData != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    transferData->OnTransferObtainedContext = transferObtainedContext;
    gBdxTransferManager.ExpectATransfer();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_Bdx_StopExpectingBdxTransfer(PyObject transferObtainedContext)
{
    TransferData * transferData = gTransfers.TransferDataForTransferObtainedContext(transferObtainedContext);
    VerifyOrReturnValue(transferData != nullptr, ToPyChipError(CHIP_ERROR_NOT_FOUND));
    gBdxTransferManager.StopExpectingATransfer();
    gTransfers.RemoveTransferData(transferData);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_Bdx_AcceptSendTransfer(chip::bdx::BdxTransfer * transfer, PyObject dataReceivedContext,
                                          PyObject transferCompletedContext)
{
    TransferData * transferData = gTransfers.TransferDataForTransfer(transfer);
    transferData->OnDataReceivedContext = dataReceivedContext;
    transferData->OnTransferCompletedContext = transferCompletedContext;
    return ToPyChipError(transfer->AcceptSend());
}

PyChipError pychip_Bdx_AcceptReceiveTransfer(chip::bdx::BdxTransfer * transfer, const uint8_t * dataBuffer, size_t dataLength,
                                             PyObject transferCompletedContext)
{
    TransferData * transferData = gTransfers.TransferDataForTransfer(transfer);
    transferData->OnTransferCompletedContext = transferCompletedContext;
    chip::ByteSpan data(dataBuffer, dataLength);
    return ToPyChipError(transfer->AcceptReceive(data));
}

PyChipError pychip_Bdx_RejectTransfer(chip::bdx::BdxTransfer * transfer)
{
    return ToPyChipError(transfer->Reject());
}

}
