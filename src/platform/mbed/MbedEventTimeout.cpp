#include "MbedEventTimeout.h"
#include <drivers/Timeout.h>

namespace chip {
namespace DeviceLayer {

using namespace mbed;

static Timeout mTimeout;

void MbedEventTimeout::AttachTimeout(Callback<void()> func, std::chrono::microseconds t)
{
    mTimeout.attach(func, t);
}

void MbedEventTimeout::DetachTimeout()
{
    mTimeout.detach();
}

} // namespace DeviceLayer
} // namespace chip
