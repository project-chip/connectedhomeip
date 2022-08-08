#ifndef __BCT_H__
#define __BCT_H__

#include <errno.h>
#include "dns_sd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"
#include "lwip/sockets.h"
#include "mDNSDebug.h"
#include "nvdm.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

// for BCT
// register Bonjour service
void reg_service_cmd(char** argv);
// rename Bonjour service as "New - Bonjour Service Name"
void rename_service_cmd(char **argv);

#ifdef __cplusplus
}
#endif

#endif /* __BCT_H__ */

