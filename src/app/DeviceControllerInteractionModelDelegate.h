#pragma once

#include <cstdlib>

#include <app/BufferedReadCallback.h>
#include <app/CommandSender.h>
#include <app/InteractionModelDelegate.h>
#include <app/util/error-mapping.h>
#include <app/util/im-client-callbacks.h>

namespace chip {
namespace Controller {
/**
 * @brief
 * Used for make current OnSuccessCallback & OnFailureCallback works when interaction model landed, it will be removed
 * after #6308 is landed.
 *
 * TODO:(#8967) Implementation of CommandSender::Callback should be removed after switching to ClusterObjects.
 */
class DeviceControllerInteractionModelDelegate : public chip::app::ReadClient::Callback,
                                                 public chip::app::CommandSender::Callback,
                                                 public chip::app::WriteClient::Callback,
                                                 public chip::app::InteractionModelDelegate
{
public:
    DeviceControllerInteractionModelDelegate() : mBufferedReadAdapter(*this) {}

    app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath,
                    const chip::app::StatusIB & aStatus, TLV::TLVReader * aData) override
    {
        // Generally IM has more detailed errors than ember library, here we always use the, the actual handling of the
        // commands should implement full IMDelegate.
        // #6308 By implement app side IM delegate, we should be able to accept detailed error codes.
        // Note: The IMDefaultResponseCallback is a bridge to the old CallbackMgr before IM is landed, so it still accepts
        // EmberAfStatus instead of IM status code.
        if (aData != nullptr)
        {
            chip::app::DispatchSingleClusterResponseCommand(aPath, *aData, apCommandSender);
        }
        else
        {
            IMDefaultResponseCallback(apCommandSender, EMBER_ZCL_STATUS_SUCCESS);
        }
    }

    void OnError(const app::CommandSender * apCommandSender, const chip::app::StatusIB & aStatus,
                 CHIP_ERROR aProtocolError) override
    {
        // The IMDefaultResponseCallback started out life as an Ember function, so it only accepted
        // Ember status codes. Consequently, let's convert the IM code over to a meaningful Ember status before dispatching.
        //
        // This however, results in loss (aError is completely discarded). When full cluster-specific status codes are implemented
        // as well, this will be an even bigger problem.
        //
        // For now, #10331 tracks this issue.
        IMDefaultResponseCallback(apCommandSender, app::ToEmberAfStatus(aStatus.mStatus));
    }

    void OnDone(app::CommandSender * apCommandSender) override { return chip::Platform::Delete(apCommandSender); }

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath,
                    app::StatusIB attributeStatus) override
    {
        IMWriteResponseCallback(apWriteClient, attributeStatus.mStatus);
    }

    void OnError(const app::WriteClient * apWriteClient, const app::StatusIB & aStatus, CHIP_ERROR aError) override
    {
        IMWriteResponseCallback(apWriteClient, aStatus.mStatus);
    }

    void OnDone(app::WriteClient * apWriteClient) override {}

    void OnEventData(const app::ReadClient * apReadClient, const app::EventHeader & aEventHeader, TLV::TLVReader * apData,
                     const app::StatusIB * apStatus) override
    {}

    void OnAttributeData(const app::ReadClient * apReadClient, const app::ConcreteDataAttributePath & aPath,
                         TLV::TLVReader * apData, const app::StatusIB & aStatus) override
    {
        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());

        IMReadReportAttributesResponseCallback(apReadClient, &aPath, apData, aStatus.mStatus);
    }

    void OnSubscriptionEstablished(const app::ReadClient * apReadClient) override
    {
        IMSubscribeResponseCallback(apReadClient, EMBER_ZCL_STATUS_SUCCESS);
    }

    void OnError(const app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        app::ClusterInfo path;
        path.mNodeId = apReadClient->GetPeerNodeId();
        IMReadReportAttributesResponseCallback(apReadClient, nullptr, nullptr, Protocols::InteractionModel::Status::Failure);
    }

    void OnDone(app::ReadClient * apReadClient) override
    {
        if (apReadClient->IsSubscriptionType())
        {
            this->FreeAttributePathParam(reinterpret_cast<uint64_t>(apReadClient));
        }
    }

    // TODO: FreeAttributePathParam and AllocateAttributePathParam are used by CHIPDevice.cpp for getting a long-live attribute path
    // object.
    void FreeAttributePathParam(uint64_t applicationId)
    {
        for (auto & item : mAttributePathTransactionMapPool)
        {
            if (item.ApplicationId == applicationId)
            {
                item.ApplicationId = UINT64_MAX;
            }
        }
    }

    // TODO: We only support allocating one path, should support multiple path later.
    app::AttributePathParams * AllocateAttributePathParam(size_t n, uint64_t applicationId)
    {
        if (n > 1)
        {
            return nullptr;
        }
        for (auto & item : mAttributePathTransactionMapPool)
        {
            if (item.ApplicationId == UINT64_MAX)
            {
                item.ApplicationId = applicationId;
                return &item.Params;
            }
        }
        return nullptr;
    }

private:
    struct AttributePathTransactionMap
    {
        uint64_t ApplicationId = UINT64_MAX;
        app::AttributePathParams Params;
    };

    app::BufferedReadCallback mBufferedReadAdapter;
    AttributePathTransactionMap mAttributePathTransactionMapPool[CHIP_DEVICE_CONTROLLER_SUBSCRIPTION_ATTRIBUTE_PATH_POOL_SIZE];
};

} // namespace Controller
} // namespace chip
