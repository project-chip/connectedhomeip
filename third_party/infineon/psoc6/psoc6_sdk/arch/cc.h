#pragma once

#include <errno.h>
#include <time.h>
#define LWIP_PLATFORM_ASSERT(x)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);                                               \
        abort();                                                                                                                   \
    } while (0)

#ifdef __ICCARM__
#define PACK_STRUCT_BEGIN __packed
#endif
