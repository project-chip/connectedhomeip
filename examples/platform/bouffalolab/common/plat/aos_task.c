
/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */

#include <FreeRTOS.h>
#include <timers.h>

#ifdef SYS_AOS_LOOP_ENABLE
#include <vfs.h>

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>

#include <AppConfig.h>

static void aos_loop_proc(void * pvParameters)
{
    aos_loop_init();

    aos_loop_run();

    vTaskDelete(NULL);
}

void aos_loop_start(void)
{
    static TaskHandle_t aos_loop_proc_task;

    vfs_init();
    vfs_device_init();

    xTaskCreate(aos_loop_proc, (char *) "event_loop", 4096, NULL, APP_TASK_PRIORITY, &aos_loop_proc_task);
}
#endif
