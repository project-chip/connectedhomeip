#include <stdio.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <stream_buffer.h>
#include <task.h>

#define STREAM_BUFF_SIZE 512

StreamBufferHandle_t xStreamBuffer;
static int enable_async_log_flag = 0;

typedef void (*pop_callback)(void *, void * src, const uint16_t len);

int uring_fifo_init()
{
    xStreamBuffer = xStreamBufferCreate(STREAM_BUFF_SIZE, 1);

    if (xStreamBuffer == NULL)
    {
        return -1;
    }

    return 0;
}

int async_log_push(const void * buf, const uint16_t len)
{
    int xBytesSent;

    const TickType_t xBlockTime = pdMS_TO_TICKS(10);
    xBytesSent                  = xStreamBufferSend(xStreamBuffer, (void *) buf, len, 0);

    if (xBytesSent != len)
    {
        return -1;
    }
}

int async_log_pop_cb(pop_callback cb, void * cb_arg)
{
    char tmp_buf[256];
    size_t rcv_size = 0;

    rcv_size = xStreamBufferReceive(xStreamBuffer, (void *) tmp_buf, sizeof(tmp_buf), portMAX_DELAY);

    if (cb != NULL)
    {
        cb(cb_arg, tmp_buf, (uint16_t) rcv_size);
    }

    return 0;
}
static void log_handler(void * para, void * log_text, const uint16_t log_len)
{
    if ((log_text != NULL) && log_len > 0)
    {
        puts(log_text);
    }
}

static void log_routine(void * para)
{
    while (1)
    {
        async_log_pop_cb(log_handler, NULL);
    }
}

void log_async_init(void)
{
    int ret;

    TaskHandle_t async_log_task;
    ret = uring_fifo_init();
    if (ret < 0)
    {
        puts("stream buff init fail.\r\n");
    }

    xTaskCreate(log_routine, (char *) "async_log", 1024, NULL, 15, &async_log_task);
}

int async_log(void)
{
    return enable_async_log_flag;
}

void enable_async_log(void)
{
    enable_async_log_flag = 1;
}
