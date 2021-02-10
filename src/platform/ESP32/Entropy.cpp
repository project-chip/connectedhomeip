/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides implementations for the CHIP entropy sourcing functions
 *          on the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/crypto/CHIPRNG.h>

#include "esp_log.h"

using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

int GetEntropy_ESP32(uint8_t * buf, size_t bufSize)
{
    while (bufSize > 0)
    {
        union
        {
            uint32_t asInt;
            uint8_t asBytes[sizeof(asInt)];
        } rnd;

        rnd.asInt = esp_random();

        size_t n = chip::min(bufSize, sizeof(rnd.asBytes));

        memcpy(buf, rnd.asBytes, n);

        buf += n;
        bufSize -= n;
    }

    return 0;
}

} // unnamed namespace

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;
    unsigned int seed;

    // Initialize the source used by Chip to get secure random data.
    err = ::chip::Platform::Security::InitSecureRandomDataSource(GetEntropy_ESP32, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    err = ::chip::Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
    SuccessOrExit(err);
    srand(seed);
    ESP_LOGI(TAG, "srand seed set: %u", seed);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "InitEntropy() failed: %s", ErrorStr(err));
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
