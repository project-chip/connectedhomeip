/* See Project CHIP LICENSE file for licensing information. */

#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <cinttypes>
#include <cstdio>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef USE_SYSLOG
#include <syslog.h>
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput() {}

} // namespace DeviceLayer

namespace Logging {
namespace Platform {

// ================================================================================
// Main Code
// ================================================================================

// SSM: start
// LSHandle *Handle = NULL;

// int initialize_service_ls2_calls();

// static void printError(LSError& lserror)
// {
//     LSErrorPrint(&lserror, stdout);
// }

// static bool handle_hello(LSHandle *sh, LSMessage* message, void* ctx)
// {
//     if (!LSCall(sh, "palm://com.webos.service.bluetooth2/adapter/getStatus", "{}", NULL, NULL, NULL, NULL))
//     {
//         g_print("Failed to call LSCall\n");
//     }
//     g_print("Hello\n");
//     std::string answer = "{\"returnValue\": true}";
//     LSMessageReply(sh, message, answer.c_str(), NULL);
//     return true;
// }


// int initialize_service_ls2_calls()
// {
//     LSMethod serviceMethods[] = {
//                 { "hello", handle_hello },
//                 { },
//     };

//     Handle = NULL;
//     GMainLoop *mainloop = g_main_loop_new(NULL, false);
//     LSError lserror;
//     LSErrorInit(&lserror);

//     if (!LSRegister("com.webos.service.matter", &Handle, &lserror))
//     {
//         g_print("Unable to register to luna-bus\n");
//         printError(lserror);
//         LSErrorFree(&lserror);
//         return -1;
//     }

// 	if (!LSRegisterCategory(Handle, "/", serviceMethods, NULL, NULL, &lserror))
//     {
//         g_print("Unable to register category and method\n");
//         printError(lserror);
//         LSErrorFree(&lserror);
//         return -2;
//     }

//     if (!LSGmainAttach(Handle, mainloop, &lserror))
//     {
//         g_print("Unable to attach service\n");
//         printError(lserror);
//         LSErrorFree(&lserror);
//         return -1;
//     }
//     g_main_loop_run(mainloop);
//     g_main_loop_unref(mainloop);
//     return 0;
// }

/**
 * CHIP log output functions.
 */
void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    //  initialize_service_ls2_calls();
    struct timeval tv;

    // Should not fail per man page of gettimeofday(), but failed to get time is not a fatal error in log. The bad time value will
    // indicate the error occurred during getting time.
    gettimeofday(&tv, nullptr);

#ifdef USE_SYSLOG
    vsyslog(category, msg, v);
#else
    printf("[%" PRIu64 ".%06" PRIu64 "][%lld:%lld] CHIP:%s: ", static_cast<uint64_t>(tv.tv_sec), static_cast<uint64_t>(tv.tv_usec),
           static_cast<long long>(syscall(SYS_getpid)), static_cast<long long>(syscall(SYS_gettid)), module);
    vprintf(msg, v);
    printf("\n");
    fflush(stdout);
#endif

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
