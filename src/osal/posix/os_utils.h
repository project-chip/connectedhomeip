
#ifndef _OS_UTILS_H
#define _OS_UTILS_H

#include <chip/osal.h>
#include <nlassert.h>

#define SuccessOrExit(aStatus) nlEXPECT((aStatus) == CHIP_OS_OK, exit)

#define VerifyOrExit(aCondition, anAction) nlEXPECT_ACTION(aCondition, exit, anAction)

chip_os_error_t map_posix_to_osal_error(int ret);

#endif // _OS_UTILS_H
