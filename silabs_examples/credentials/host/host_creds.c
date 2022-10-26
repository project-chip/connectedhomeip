#include "host_creds.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#define CREDS_CSR_LENGTH_MAX 1024
#define CREDS_COMMON_NAME_MAX 128
#define CREDS_REQ_HEADER_SIZE 5
#define CREDS_RES_HEADER_SIZE 4

static int serial_port_open(const char * port) {

    struct termios tty;

    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -2;
    }

    tty.c_cflag |= CREAD | CLOCAL; // Allows read, and ignore control lines
    tty.c_lflag &= ~ICANON; // Non-canonical mode
    tty.c_lflag &= ~ISIG;   // Ignore signal characters (INTR, QUIT, SUSP, or DSUSP)
    tty.c_iflag &= ~(INLCR | ICRNL);  // Disable mapping NL to CR-NL on input
    tty.c_cc[VTIME] = 10;   // Wait up to 1 second
    tty.c_cc[VMIN] = 0;     // Minimum number of characters for noncanonical

    // Save tty settings, also checking for error
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return -3;
    }

    return fd;
}

static int creds_file_write(const char * path, const char * data, size_t size)
{
    printf("Writting '%s' (%zu)...\n", path, size);
    FILE * fp = fopen(path, "w+");
    fwrite(data, size, 1, fp);
    fclose(fp);
    return 0;
}

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

int host_creds_csr(const char * port, const char *common_name, uint32_t vendor_id, uint32_t product_id, const char * filename)
{
    printf("Requesting CSR (cn:'%s',vid:0x%04x, pid:0x%04x) from '%s' as '%s'\n", common_name, vendor_id, product_id, port, filename);

    uint8_t buffer[CREDS_CSR_LENGTH_MAX] = { 0 };
    int incoming                      = 0;
    int outgoing                      = 0;
    size_t offset                     = 0;

    // Open serial port

    int fd = serial_port_open(port);
    if (fd < 0)
    {
        printf("Serial port open error %d\n", fd);
        return -1;
    }

    // Send CSR request

    printf("Requesing CSR...\n");
    size_t cn_size = strnlen(common_name, CREDS_COMMON_NAME_MAX) + 1;
    buffer[0] = (vendor_id >> 8) & 0xff;
    buffer[1] = (vendor_id & 0xff);
    buffer[2] = (product_id >> 8) & 0xff;
    buffer[3] = (product_id & 0xff);
    buffer[4] = (uint8_t) cn_size;
    memcpy(&buffer[CREDS_REQ_HEADER_SIZE], common_name, cn_size);

    while (offset < CREDS_REQ_HEADER_SIZE + cn_size)
    {
        outgoing += write(fd, buffer, CREDS_REQ_HEADER_SIZE + cn_size);
        if(outgoing < 0)
        {
            printf("Serial write error %d %s\n", errno, strerror(errno));
            return -2;
        }
        offset += outgoing;
    }

    // Read header
    offset = 0;
    do
    {
        incoming = read(fd, &buffer[offset], CREDS_CSR_LENGTH_MAX - offset);
        if ((incoming > 0) && ((size_t) incoming < CREDS_CSR_LENGTH_MAX - offset))
        {
            offset += incoming;
            if (0 == buffer[offset - 1])
            {
                // Done
                break;
            }
        }
    } while (offset < CREDS_RES_HEADER_SIZE);

    // Decode header: error(2), size(2)
    int16_t err = (buffer[0] << 8) | buffer[1];
    uint32_t expected_size = (buffer[2] << 8) | buffer[3];
    uint32_t received_size = offset - CREDS_RES_HEADER_SIZE;
    if (err || expected_size > CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE)
    {
        printf("CSR generation error: %d\n", err);
        return err;
    }

    printf("CSR generated, size:%u\n", expected_size);

    // Read file
    uint8_t *csr = &buffer[CREDS_RES_HEADER_SIZE];
    do
    {
        incoming = read(fd, &csr[received_size], CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE - received_size);
        if ((incoming > 0) && ((size_t) incoming <= CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE - received_size))
        {
            received_size += incoming;
            // printf("Reading CSR: %d / %zu\n", incoming, offset);
            if (0 == csr[received_size - 1])
            {
                // Done
                break;
            }
        }
    } while (received_size < (CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE));

    // Store file
    if (received_size > CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE)
    {
        printf("Invalid file size: %u > %u\n", received_size, CREDS_CSR_LENGTH_MAX - CREDS_RES_HEADER_SIZE);
        return -1;
    }

    creds_file_write(filename, (const char *)csr, received_size);

    return 0;
}
