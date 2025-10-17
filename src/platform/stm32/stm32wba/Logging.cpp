/* See Project CHIP LICENSE file for licensing information. */

#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <stdio.h>
#include "log_module.h"
#include "utilities_conf.h"

#define CHIP_STR "CHIP:"
#define ENDOFLINE_CHAR "\n"
#define DOT ":"

namespace chip {
namespace Logging {
namespace Platform {

void ENFORCE_FORMAT(3, 0) LogV(const char *module, uint8_t category,
		const char *msg, va_list v) {
	if (msg != NULL) {
		Log_Module_Print(LOG_VERBOSE_INFO, LOG_REGION_APP, CHIP_STR);
		if (module != NULL) {
			Log_Module_Print(LOG_VERBOSE_INFO, LOG_REGION_APP, module);
			Log_Module_Print(LOG_VERBOSE_INFO, LOG_REGION_APP, DOT);
		}
		Log_Module_PrintWithArg(LOG_VERBOSE_INFO, LOG_REGION_APP, msg, v);
		Log_Module_Print(LOG_VERBOSE_INFO, LOG_REGION_APP, ENDOFLINE_CHAR);
	}
}

} // namespace Platform
} // namespace Logging
} // namespace chip
