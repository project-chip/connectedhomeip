
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <controller/CHIPDeviceController.h>


using namespace chip;
using namespace chip::Controller;

class Draft
{
public:
    Draft() : mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this){}
        
    CHIP_ERROR Run();
    CHIP_ERROR Setup(Context &ctx);
    CHIP_ERROR Start(Draft &d, Context &ctx);
    CHIP_ERROR Pair(Context &ctx, NodeId remote_id, Transport::PeerAddress peer_addr, uint32_t pin_code, uint16_t discriminator);


    static void OnDeviceConnectedFn(void * context, chip::Controller::Device * device);
    static void OnDeviceConnectionFailureFn(void * context, chip::NodeId deviceId, CHIP_ERROR error);

    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};