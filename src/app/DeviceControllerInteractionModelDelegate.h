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
class DeviceControllerInteractionModelDelegate : public chip::app::CommandSender::Callback,
                                                 public chip::app::WriteClient::Callback,
                                                 public chip::app::InteractionModelDelegate
{
public:
    DeviceControllerInteractionModelDelegate() {}

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

    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        // The IMDefaultResponseCallback started out life as an Ember function, so it only accepted
        // Ember status codes. Consequently, let's convert the error over to a meaningful Ember status before dispatching.
        //
        // This however, results in loss (non-IM errors and cluster-specific
        // status codes are completely discarded).
        //
        // For now, #10331 tracks this issue.
        app::StatusIB status(aError);
        IMDefaultResponseCallback(apCommandSender, app::ToEmberAfStatus(status.mStatus));
    }

    void OnDone(app::CommandSender * apCommandSender) override { return chip::Platform::Delete(apCommandSender); }

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath,
                    app::StatusIB attributeStatus) override
    {
        IMWriteResponseCallback(apWriteClient, attributeStatus.mStatus);
    }

    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        app::StatusIB status(aError);
        IMWriteResponseCallback(apWriteClient, status.mStatus);
    }

    void OnDone(app::WriteClient * apWriteClient) override {}
};

} // namespace Controller
} // namespace chip
