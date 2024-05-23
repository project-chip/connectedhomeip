/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <inttypes.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceConfig.h>
#include <src/lib/support/CodeUtils.h>

#include "fsl_debug_console.h"
#include <cstring>

#define K32W_LOG_MODULE_NAME chip
#define EOL_CHARS "\r\n" /* End of Line Characters */
#define EOL_CHARS_LEN 2  /* Length of EOL */

/* maximum value for uint32_t is 4294967295 - 10 bytes + 2 bytes for the brackets */
static constexpr uint8_t timestamp_max_len_bytes = 12;

/* one byte for the category + 2 bytes for the brackets */
static constexpr uint8_t category_max_len_bytes = 3;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <utils/uart.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

extern "C" uint32_t otPlatAlarmMilliGetNow(void);

namespace chip {
namespace Logging {
namespace Platform {

void GetMessageString(char * buf, uint8_t bufLen, const char * module, uint8_t category)
{
    int writtenLen = 0;
    const char * categoryString;

    /* bufLen must accommodate the length of the timestamp + length of the category +
     * length of the module + 2 bytes for the module's brackets +
     * 1 byte for the terminating character.
     */
    assert(bufLen >= (timestamp_max_len_bytes + category_max_len_bytes + (strlen(module) + 2) + 1));

    writtenLen = snprintf(buf, bufLen, "[%ld]", otPlatAlarmMilliGetNow());
    bufLen -= writtenLen;
    buf += writtenLen;

    if (category != kLogCategory_None)
    {
        switch (category)
        {
        case kLogCategory_Error:
            categoryString = "E";
            break;
        case kLogCategory_Progress:
            categoryString = "P";
            break;
        case kLogCategory_Detail:
            categoryString = "D";
            break;
        default:
            categoryString = "U";
        }

        writtenLen = snprintf(buf, bufLen, "[%s]", categoryString);
        bufLen -= writtenLen;
        buf += writtenLen;
    }

    writtenLen = snprintf(buf, bufLen, "[%s]", module);
}

} // namespace Platform
} // namespace Logging
} // namespace chip

void FillPrefix(char * buf, uint8_t bufLen, const char * module, uint8_t category)
{
    chip::Logging::Platform::GetMessageString(buf, bufLen, module, category);
}

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by CHIP or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

void ENFORCE_FORMAT(1, 0) GenericLog(const char * format, va_list arg, const char * module, uint8_t category)
{

#if K32W_LOG_ENABLED

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1] = { 0 };
    size_t prefixLen, writtenLen;

    /* Prefix is composed of [Time Reference][Debug String][Module Name String] */
    FillPrefix(formattedMsg, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1, module, category);
    prefixLen = strlen(formattedMsg);

    // Append the log message.
    writtenLen = vsnprintf(formattedMsg + prefixLen, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - prefixLen - EOL_CHARS_LEN, format, arg);
    VerifyOrDie(writtenLen > 0);
    memcpy(formattedMsg + prefixLen + writtenLen, EOL_CHARS, EOL_CHARS_LEN);

    PRINTF("%s", formattedMsg);

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();

#endif // K32W_LOG_ENABLED
}

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output function.
 */
void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

#if K32W_LOG_ENABLED
    GenericLog(msg, v, module, category);
    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // K32W_LOG_ENABLED
}

} // namespace Platform
} // namespace Logging
} // namespace chip

#undef K32W_LOG_MODULE_NAME
#define K32W_LOG_MODULE_NAME lwip

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/**
 * LwIP log output function.
 */
extern "C" void ENFORCE_FORMAT(1, 2) LwIPLog(const char * msg, ...)
{

#if K32W_LOG_ENABLED
    va_list v;
    const char * module = "LWIP";

    va_start(v, msg);
    GenericLog(msg, v, module, chip::Logging::kLogCategory_None);
    va_end(v);
#endif
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#undef K32W_LOG_MODULE_NAME
#define K32W_LOG_MODULE_NAME thread

extern "C" void ENFORCE_FORMAT(3, 4) otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{

#if K32W_LOG_ENABLED
    va_list v;
    const char * module = "OT";

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);
    GenericLog(aFormat, v, module, chip::Logging::kLogCategory_None);
    va_end(v);
#endif
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
