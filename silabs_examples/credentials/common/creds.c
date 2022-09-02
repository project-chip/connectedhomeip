#include "creds.h"
#include <string.h>
#include <stdlib.h>

int creds_file_init(creds_file_t *file, uint8_t *buffer, size_t buffer_max)
{
    memset(file, 0x00, sizeof(creds_file_t));
    file->data = buffer;
    file->max = buffer_max;
    return 0;
}
