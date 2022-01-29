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
class DeviceControllerInteractionModelDelegate : public chip::app::WriteClient::Callback
{
public:
    DeviceControllerInteractionModelDelegate() {}

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
