#include "host_creds.h"
#include <zigbee_app_framework_common.h>
#include <stack/include/ember.h>
#include <app/util/ezsp/ezsp-protocol.h>

EmberStatus ezspCustomFrame(
    uint8_t payloadLength,
    uint8_t *payload,
    uint8_t *replyLength,
    uint8_t *reply);

static int creds_file_write(creds_file_t *file, const char *path)
{
    printf("Writting '%s' (%zu)...\n", path, file->size);
    FILE *fp = fopen(path, "w+");
    fwrite(file->data, file->size, 1, fp);
    fclose(fp);
    return 0;
}

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

int host_creds_csr()
{
    printf("\n******** CSR REQUEST ********\n");

    uint8_t payload[CREDS_FRAME_LENGTH_MAX];
    uint8_t reply[CREDS_FRAME_LENGTH_MAX];
    uint8_t reply_size = sizeof(reply);
    uint8_t csr_buffer[CREDS_CSR_LENGTH_MAX];
    creds_file_t csr;
    EmberStatus status;
    int err = 0;

    ASSERT(!err, return err, "PAI read error %d", err);

    // CSR Request
    printf("Generating CSR...\n");
    creds_file_init(&csr, csr_buffer, sizeof(csr_buffer));
    payload[0] = CREDS_COMMAND_CSR_GENERATE;
    reply_size = sizeof(reply);
    status = ezspCustomFrame(1, payload, &reply_size, reply);
    ASSERT(!status && (2 == reply_size), return err, "CSR request error %d", err);

    size_t csr_size = ((reply[0] << 8) | reply[1]);
    ASSERT(csr_size <= csr.max, return -1, "Invalid size: %zu > %zu\n", csr_size, csr.max);

    // Read CSR
    printf("Reading CSR...\n");
    payload[0] = CREDS_COMMAND_CSR_READ;
    while (EMBER_SUCCESS == status && csr.offset < csr_size)
    {
        reply_size = sizeof(reply);
        status = ezspCustomFrame(1, payload, &reply_size, reply);
        if (!status && (reply_size > 0) && (csr.offset + reply_size <= csr_size))
        {
            memcpy(&csr.data[csr.offset], reply, reply_size);
            csr.offset += reply_size;
        }
    }

    if (EMBER_SUCCESS == status && csr.offset > 0)
    {
        csr.size = csr.offset;
        creds_file_write(&csr, host_option_get(APP_OPTION_CSR_FILENAME));
        return 0;
    }

    printf("Status: 0x%02x (%d)\n", status, status);
    return 1;
}
