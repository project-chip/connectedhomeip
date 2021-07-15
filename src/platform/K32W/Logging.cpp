/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>
#include <src/lib/support/CodeUtils.h>
#include <support/logging/Constants.h>

#include <cstring>

#define K32W_LOG_MODULE_NAME chip
#define EOL_CHARS "\r\n" /* End of Line Characters */
#define EOL_CHARS_LEN 2  /* Length of EOL */

#define TIMESTAMP_MAX_LEN_BYTES 10
#define CATEGORY_MAX_LEN_BYTES 1

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <utils/uart.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

static bool isLogInitialized;
extern uint8_t gOtLogUartInstance;
extern "C" void K32WWriteBlocking(const uint8_t * aBuf, uint32_t len);
extern "C" uint32_t otPlatAlarmMilliGetNow(void);

namespace chip {
namespace Logging {
namespace Platform {

/* bufLen must have at least 4 (miliseconds timestamp) + 1 (category) +  */
void GetMessageString(char * buf, uint8_t bufLen, const char * module, uint8_t category)
{
    int writtenLen = 0;
    const char * categoryString;

    assert(bufLen >= (TIMESTAMP_MAX_LEN_BYTES + CATEGORY_MAX_LEN_BYTES + strlen(module) + 1));

    writtenLen = snprintf(buf, bufLen, "[%lu]", otPlatAlarmMilliGetNow());
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

    writtenLen = snprintf(buf + writtenLen, bufLen, "[%s]", module);
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

void GenericLog(const char * format, va_list arg, const char * module, uint8_t category)
{

#if K32W_LOG_ENABLED

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1] = { 0 };
    size_t prefixLen, writtenLen;

    if (!isLogInitialized)
    {
        isLogInitialized   = true;
        gOtLogUartInstance = 0;
        otPlatUartEnable();
    }

    /* Prefix is composed of [Time Reference][Debug String][Module Name String] */
    FillPrefix(formattedMsg, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - 1, module, category);
    prefixLen = strlen(formattedMsg);

    // Append the log message.
    writtenLen = vsnprintf(formattedMsg + prefixLen, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE - prefixLen - EOL_CHARS_LEN, format, arg);
    VerifyOrDie(writtenLen > 0);
    memcpy(formattedMsg + prefixLen + writtenLen, EOL_CHARS, EOL_CHARS_LEN);

    K32WWriteBlocking((const uint8_t *) formattedMsg, strlen(formattedMsg));

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
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
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

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;
    const char * module = "LWIP";

    va_start(v, msg);
    GenericLog(msg, v, module, chip::Logging::kLogCategory_None);
    va_end(v);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#undef K32W_LOG_MODULE_NAME
#define K32W_LOG_MODULE_NAME thread

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list v;
    const char * module = "OT";

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);
    GenericLog(aFormat, v, module, chip::Logging::kLogCategory_None);
    va_end(v);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
