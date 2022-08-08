#ifndef __MDNS_PORTING_H__
#define __MDNS_PORTING_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "type_def.h"

// For KEIL/IAR IDE
#if (defined(__CC_ARM) || defined(__ICCARM__))

#ifndef pid_t_defined
typedef int pid_t;
#define pid_t_defined
#endif

#ifndef __u_char_defined
typedef unsigned char   u_char;
#define __u_char_defined
#endif
#ifndef __u_short_defined
typedef unsigned short  u_short;
#define __u_short_defined
#endif
#ifndef __u_int_defined
typedef unsigned int    u_int;
#define __u_int_defined
#endif
#ifndef __u_long_defined
typedef unsigned long   u_long;
#define __u_long_defined
#endif

#elif defined(__GNUC__)
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MDNS_PORTING_H__ */

