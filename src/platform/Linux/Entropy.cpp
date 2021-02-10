/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides implementations for the chip entropy sourcing functions
 *          on the Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/crypto/CHIPRNG.h>

using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;
    unsigned int seed;

    // Initialize the source used by CHIP to get secure random data.
    err = Platform::Security::InitSecureRandomDataSource(getentropy, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    err = Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
    SuccessOrExit(err);
    srand(seed);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "InitEntropy() failed: %d" err);
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
