#include <FreeRTOS.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <bl_chip.h>
#include <bl_cks.h>
#include <bl_dma.h>
#include <bl_irq.h>
#include <bl_sec.h>
#include <bl_sys.h>
#include <bl_sys_time.h>
#include <bl_uart.h>
#include <bl_wifi.h>
#include <blog.h>
#include <cli.h>
#include <device/vfs_uart.h>
#include <event_device.h>
#include <fdt.h>
#include <hal_board.h>
#include <hal_boot2.h>
#include <hal_sys.h>
#include <hosal_uart.h>
#include <libfdt.h>
#include <stdio.h>
#include <task.h>
#include <timers.h>
#include <vfs.h>
#ifdef EASYFLASH_ENABLE
#include <easyflash.h>
#endif
#ifdef SYS_LOOPRT_ENABLE
#include <looprt.h>
#include <loopset.h>
#endif
#ifdef SYS_USER_VFS_ROMFS_ENABLE
#include <bl_romfs.h>
#endif

HOSAL_UART_DEV_DECL(uart_stdio, 0, 16, 7, 115200);

extern uint8_t _heap_start;
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (unsigned int) &_heap_size }, // set on runtime
    { &_heap_wifi_start, (unsigned int) &_heap_wifi_size },
    { NULL, 0 }, /* Terminates the array. */
    { NULL, 0 }  /* Terminates the array. */
};

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    puts("Stack Overflow checked\r\n");
    while (1)
    {
        /*empty here*/
    }
}

void __attribute__((weak)) vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n", xPortGetFreeHeapSize());
    while (1)
    {
        /*empty here*/
    }
}

void __attribute__((weak)) vApplicationIdleHook(void)
{
    __asm volatile("   wfi     ");
    /*empty*/
}

void __attribute__((weak)) vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer,
                                                         StackType_t ** ppxIdleTaskStackBuffer, uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                    uint32_t * pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void user_vAssertCalled(void) __attribute__((weak, alias("vAssertCalled")));
void __attribute__((weak)) vAssertCalled(void)
{
    taskDISABLE_INTERRUPTS();
    abort();
}

#ifdef BL602_MATTER_SUPPORT
void setup_heap(void)
{
    // Invoked during system boot via start.S
    vPortDefineHeapRegions(xHeapRegions);
}
#endif

#ifdef SYS_VFS_UART_ENABLE
static int get_dts_addr(const char * name, uint32_t * start, uint32_t * off)
{
    uint32_t addr    = hal_board_get_factory_addr();
    const void * fdt = (const void *) addr;
    uint32_t offset;

    if (!name || !start || !off)
    {
        return -1;
    }

    offset = fdt_subnode_offset(fdt, 0, name);
    if (offset <= 0)
    {
        log_error("%s NULL.\r\n", name);
        return -1;
    }

    *start = (uint32_t) fdt;
    *off   = offset;

    return 0;
}
#endif

static void app_main_entry(void * pvParameters)
{
    extern int main();
    main();
    vTaskDelete(NULL);
}

static void aos_loop_proc(void * pvParameters)
{
#ifdef SYS_LOOPRT_ENABLE
    static StackType_t proc_stack_looprt[512];
    static StaticTask_t proc_task_looprt;

    /*Init bloop stuff*/
    looprt_start(proc_stack_looprt, 512, &proc_task_looprt);
    loopset_led_hook_on_looprt();
#endif
#ifdef EASYFLASH_ENABLE
    easyflash_init();
#endif

#ifdef SYS_VFS_ENABLE
    vfs_init();
    vfs_device_init();
#endif

#ifdef SYS_VFS_UART_ENABLE
    uint32_t fdt = 0, offset = 0;
    const char * uart_node[] = {
        "uart@4000A000",
        "uart@4000A100",
    };

    if (0 == get_dts_addr("uart", &fdt, &offset))
    {
        vfs_uart_init(fdt, offset, uart_node, 2);
    }
#endif

#ifdef SYS_USER_VFS_ROMFS_ENABLE
    romfs_register();
#endif

#ifdef SYS_AOS_LOOP_ENABLE
    aos_loop_init();
#endif

#ifdef SYS_AOS_CLI_ENABLE
    int fd_console;
    fd_console = aos_open("/dev/ttyS0", 0);
    if (fd_console >= 0)
    {
        printf("Init CLI with event Driven\r\n");
        aos_cli_init(0);
        aos_poll_read_fd(fd_console, aos_cli_event_cb_read_get(), (void *) 0x12345678);
    }
#endif

    xTaskCreate(app_main_entry, (char *) "main", SYS_APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, SYS_APP_TASK_PRIORITY, NULL);

#ifdef SYS_AOS_LOOP_ENABLE
    aos_loop_run();
#endif
    puts("------------------------------------------\r\n");
    puts("+++++++++Critical Exit From AOS LOOP entry++++++++++\r\n");
    puts("******************************************\r\n");
    vTaskDelete(NULL);
}

static void _dump_boot_info(void)
{
    char chip_feature[40];
    const char * banner;

    puts("Booting BL602 Chip...\r\n");

    /*Display Banner*/
    if (0 == bl_chip_banner(&banner))
    {
        puts(banner);
    }
    puts("\r\n");
    /*Chip Feature list*/
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
    puts("RISC-V Core Feature:");
    bl_chip_info(chip_feature);
    puts(chip_feature);
    puts("\r\n");

    puts("Build Version: ");
    puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");
    puts("Build Date: ");
    puts(__DATE__);
    puts("\r\n");
    puts("Build Time: ");
    puts(__TIME__);
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
}

static void system_early_init(void)
{
#ifdef SYS_BLOG_ENABLE
    blog_init();
#endif

    bl_irq_init();
    bl_sec_init();
    hal_boot2_init();

#ifdef SYS_DMA_ENABLE
    hosal_dma_init();
#endif

    /* To be added... */

    /* board config is set after system is init*/
    hal_board_cfg(0);
}

void bfl_main()
{
    TaskHandle_t aos_loop_proc_task;

    bl_sys_early_init();
    /*Init UART In the first place*/
    hosal_uart_init(&uart_stdio);
    puts("Starting bl602 now....\r\n");

    _dump_boot_info();

#ifndef BL602_MATTER_SUPPORT
    vPortDefineHeapRegions(xHeapRegions);
#endif

    system_early_init();

    puts("[OS] Starting aos_loop_proc task...\r\n");
    xTaskCreate(aos_loop_proc, (char *) "event_loop", 1024, NULL, 15, &aos_loop_proc_task);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();
}
