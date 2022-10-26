#ifndef MATTER_HOST_CREDS_H
#define MATTER_HOST_CREDS_H

#include <stddef.h>
#include <stdint.h>

int host_creds_csr(const char * port, const char *common_name, uint32_t vendor_id, uint32_t product_id, const char * filename);

#endif // MATTER_HOST_CREDS_H