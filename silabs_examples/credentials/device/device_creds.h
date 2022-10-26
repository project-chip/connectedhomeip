#ifndef MATTER_CREDS_H
#define MATTER_CREDS_H

#include <stdio.h>

#define ASSERT(cond, action, message, ...)                              \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            action;                                                     \
        }                                                               \
    } while (0)

#ifndef EFR32_CREDENTIALS_DAC_KEY_ID
#define EFR32_CREDENTIALS_DAC_KEY_ID PSA_KEY_ID_USER_MIN + 1
#endif

int device_creds_init();
void device_creds_action();

#endif // MATTER_CREDS_H
