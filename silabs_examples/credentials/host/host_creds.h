#ifndef MATTER_HOST_CREDS_H
#define MATTER_HOST_CREDS_H

#include <creds.h>

typedef enum host_option_
{
    APP_OPTION_CSR_FILENAME = 0x01,

} host_option_t;

#define APP_OPTION_COUNT 6

const char *host_option_get(host_option_t option);

int host_creds_csr();

#endif // MATTER_HOST_CREDS_H