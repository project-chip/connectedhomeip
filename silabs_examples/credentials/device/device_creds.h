#ifndef MATTER_DEVICE_CREDS_H
#define MATTER_DEVICE_CREDS_H

#include <creds.h>

#ifndef EFR32_CREDENTIALS_DAC_KEY_ID
#define EFR32_CREDENTIALS_DAC_KEY_ID PSA_KEY_ID_USER_MIN + 1
#endif

int device_creds_init();
int device_creds_action();
int device_csr_generate(creds_file_t *csr);

#endif // MATTER_DEVICE_CREDS_H