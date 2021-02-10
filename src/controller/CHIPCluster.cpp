/* See Project CHIP LICENSE file for licensing information. */

/**
 *  @file
 *    This file contains definitions for a base Cluster class. This class will
 *    be derived by various ZCL clusters supported by CHIP. The objects of the
 *    ZCL cluster class will be used by Controller applications to interact with
 *    the CHIP device.
 */

#include <controller/CHIPCluster.h>

namespace chip {
namespace Controller {

CHIP_ERROR ClusterBase::Associate(Device * device, EndpointId endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Check if the device supports mCluster at the requested endpoint

    mDevice   = device;
    mEndpoint = endpoint;
    return err;
}

void ClusterBase::Dissociate()
{
    mDevice = nullptr;
}

CHIP_ERROR ClusterBase::SendCommand(uint8_t seqNum, chip::System::PacketBufferHandle payload,
                                    Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!payload.IsNull(), err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(std::move(payload));
    SuccessOrExit(err);

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        mDevice->AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in sending cluster command. Err %d", err);
    }

    return err;
}

CHIP_ERROR ClusterBase::RequestAttributeReporting(AttributeId attributeId, Callback::Cancelable * onReportCallback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(onReportCallback != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mDevice->AddReportHandler(mEndpoint, mClusterId, attributeId, onReportCallback);

exit:
    return err;
}

} // namespace Controller
} // namespace chip
