/* See Project CHIP LICENSE file for licensing information. */

#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
const uint64_t TestDeviceId                       = 12344321;
const uint64_t TestFabricId                       = 0;
const uint8_t TestDeviceCert[]                    = { 0 };
const uint8_t TestDeviceIntermediateCACert[]      = { 0 };
const uint8_t TestDevicePrivateKey[]              = { 0 };
const uint16_t TestDeviceCertLength               = sizeof(TestDeviceCert);
const uint16_t TestDeviceIntermediateCACertLength = sizeof(TestDeviceIntermediateCACert);
const uint16_t TestDevicePrivateKeyLength         = sizeof(TestDevicePrivateKey);
#endif // CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
