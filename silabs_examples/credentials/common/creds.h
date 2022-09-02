#ifndef MATTER_CREDS_H
#define MATTER_CREDS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define ASSERT(cond, action, message, ...)                              \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            fprintf(stderr, "%s:%d:%s ", __FILE__, __LINE__, __func__); \
            fprintf(stderr, message, ##__VA_ARGS__);                    \
            printf("\n");                                               \
            action;                                                     \
        }                                                               \
    } while (0)

#define CREDS_PROTOCOL_FRAME_SIZE_MAX 64

typedef enum creds_command_
{
    CREDS_COMMAND_CSR_GENERATE = 0x00,
    CREDS_COMMAND_CSR_READ = 0x01,

} creds_command_t;


typedef struct creds_file_
{
    size_t max;
    size_t size;
    size_t offset;
    uint8_t *data;

} creds_file_t;

#define CREDS_FRAME_LENGTH_MAX    64
#define CREDS_CSR_LENGTH_MAX      512

int creds_file_init(creds_file_t *file, uint8_t *buffer, size_t buffer_max);

#endif // MATTER_CREDS_H
