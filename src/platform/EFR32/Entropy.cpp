/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides implementations for the Chip entropy sourcing functions
 *          on the Silcon Labs EFR32 platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/crypto/CHIPRNG.h>

#include <mbedtls/entropy_poll.h>

#include <em_device.h>

#if defined(_SILICON_LABS_32B_SERIES_1)
#include <openthread/platform/entropy.h>
#elif defined(_SILICON_LABS_32B_SERIES_2)
extern "C" int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen);
#else // !defined(_SILICON_LABS_32B_SERIES_1) && !defined(_SILICON_LABS_32B_SERIES_2)
#error "Unsupported EFR32 series"
#endif

using namespace ::chip;

#if !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
#error "CHIP DRBG implementation must be enabled on EFR32 platforms"
#endif // !CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Retrieve entropy from the underlying RNG source.
 *
 * This function is called by the CHIP DRBG to acquire entropy.
 */
int GetEntropy_EFR32(uint8_t * buf, size_t count)
{
    int res = 0;

    VerifyOrDie(count <= UINT16_MAX);

#if defined(_SILICON_LABS_32B_SERIES_1)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().LockThreadStack();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ThreadStackManagerImpl::IsInitialized())
    {
        ThreadStackMgr().LockThreadStack();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    otError otErr = otPlatEntropyGet(buf, (uint16_t) count);
    if (otErr != OT_ERROR_NONE)
    {
        res = CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED;
    }

#elif defined(_SILICON_LABS_32B_SERIES_2)
    size_t entropy_len = 0;
    size_t olen        = 0;

    while (entropy_len < count)
    {
        res = mbedtls_hardware_poll(NULL, buf + entropy_len, count - entropy_len, &olen);
        if (res != 0)
        {
            res = CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED;
            break;
        }

        entropy_len += olen;
    }
#else // !defined(_SILICON_LABS_32B_SERIES_1) && !defined(_SILICON_LABS_32B_SERIES_2)
#error "Unsupported EFR32 series"
#endif

    return res;
}

CHIP_ERROR InitEntropy()
{
    CHIP_ERROR err;

    // Initialize the CHIP DRBG.
    err = Platform::Security::InitSecureRandomDataSource(GetEntropy_EFR32, 64, NULL, 0);
    SuccessOrExit(err);

    // Seed the standard rand() pseudo-random generator with data from the secure random source.
    {
        unsigned int seed;
        err = Platform::Security::GetSecureRandomData((uint8_t *) &seed, sizeof(seed));
        SuccessOrExit(err);
        srand(seed);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "InitEntropy() failed: 0x%08" PRIX32, err);
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
