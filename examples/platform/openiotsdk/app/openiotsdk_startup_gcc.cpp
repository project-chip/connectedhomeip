/*
 *    Copyright (c) 2006-2022 ARM Limited
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <algorithm>
#include <cmsis.h>
#include <errno.h>
#include <new>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>

#include "bootstrap/mbed_critical.h"
#include "cmsis_os2.h"
extern "C" {
#include "hal/serial_api.h"
}

extern "C" uint32_t tfm_ns_interface_init(void);

#define CALLER_ADDR() __builtin_extract_return_addr(__builtin_return_address(0))

// Consider reducing the baudrate if the serial is used as input and characters are lost
extern "C" mdh_serial_t * get_example_serial();
#ifndef IOT_SDK_APP_SERIAL_BAUDRATE
#define IOT_SDK_APP_SERIAL_BAUDRATE 921600
#endif

// main thread declaration
// The thread object and associated stack are statically allocated
#ifndef IOT_SDK_APP_MAIN_STACK_SIZE
#define IOT_SDK_APP_MAIN_STACK_SIZE 16 * 1024
#endif
static void main_thread(void * argument);
alignas(8) static char main_thread_stack[IOT_SDK_APP_MAIN_STACK_SIZE];
alignas(8) static uint8_t main_thread_storage[100] __attribute__((section(".bss.os.thread.cb")));

// malloc mutex declaration
static osMutexId_t malloc_mutex;
alignas(8) static uint8_t malloc_mutex_obj[80];

// C runtime import: constructor initialization and main
extern "C" void __libc_init_array(void);
extern "C" int main(void);

// IOT SDK serial declarations
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

static mdh_serial_t * serial;

// stdout tx mutex declaration
static osMutexId_t tx_mutex;
alignas(8) static uint8_t tx_mutex_obj[80];

static int serial_out(const char * str, size_t len)
{
    if (str == NULL)
    {
        return -1;
    }

    if (len == 0)
    {
        return 0;
    }

    size_t written = 0;
    while (written++ < len)
    {
        mdh_serial_put_data(serial, *str++);
    }

    return len;
}

// prints while printf is not usable yet
static void bare_metal_print(const char * str)
{
    serial_out(str, strlen(str));
}

static void serial_irq_cb(void * p_instance, mdh_serial_irq_type_t event);

struct CriticalSection
{
    CriticalSection() { core_util_critical_section_enter(); }
    CriticalSection(const CriticalSection &)             = delete;
    CriticalSection & operator=(const CriticalSection &) = delete;
    ~CriticalSection() { core_util_critical_section_exit(); }
};

// RX buffer
template <size_t Size>
struct RingBuffer
{
    static_assert(Size > 0);
    RingBuffer() : head_(0), tail_(0), full_(false) {}
    void put(uint8_t data);
    bool get(uint8_t * data);
    size_t get(uint8_t * data, size_t size);
    bool empty() const;
    bool full() const;
    size_t size() const;

private:
    uint8_t buffer_[Size];
    size_t head_ = 0;
    size_t tail_ = 0;
    bool full_   = false;

    void increment(size_t & val);
    void increment(size_t & val, size_t incr);
    bool empty_() const;
    size_t size_() const;
};

// Buffer that receive data from serial
static RingBuffer<128> rx_buffer;

/*
 * This function override startup sequence. Instead of releasing control to the C runtime
 * the following operations are performed:
 *
 * - initialize the serial (low level)
 * - initialize RTOS
 * - Start the RTOS with the main thread
 */
extern "C" void mbed_sdk_init(void)
{
    serial = get_example_serial();
    mdh_serial_set_baud(serial, IOT_SDK_APP_SERIAL_BAUDRATE);

    int ret = osKernelInitialize();
    if (ret != osOK)
    {
        bare_metal_print("osKernelInitialize failed\r\n");
        abort();
    }

    // Create main thread used to run the application
    {
        osThreadAttr_t main_thread_attr = {
            .name       = "main",
            .cb_mem     = &main_thread_storage,
            .cb_size    = sizeof(main_thread_storage),
            .stack_mem  = main_thread_stack,
            .stack_size = sizeof(main_thread_stack),
            .priority   = osPriorityNormal,
        };

        osThreadId_t main_thread_id = osThreadNew(main_thread, NULL, &main_thread_attr);
        if (main_thread_id == NULL)
        {
            bare_metal_print("Main thread creation failed\r\n");
            abort();
        }
    }

    ret = osKernelStart();
    // Note osKernelStart should never return
    bare_metal_print("Kernel failed to start\r\n");
    abort();
}

/**
 * Main thread
 * - Initialize TF-M
 * - Initialize the toolchain:
 *  - Setup mutexes for malloc and environment
 *  - Construct global objects
 * - Run the main
 */
static void main_thread(void * argument)
{
    // Create Malloc mutex
    {
        osMutexAttr_t malloc_mutex_attr = { .name      = "malloc_mutex",
                                            .attr_bits = osMutexRecursive | osMutexPrioInherit,
                                            .cb_mem    = &malloc_mutex_obj,
                                            .cb_size   = sizeof(malloc_mutex_obj) };

        malloc_mutex = osMutexNew(&malloc_mutex_attr);
        if (malloc_mutex == NULL)
        {
            bare_metal_print("Failed to initialize malloc mutex\r\n");
            abort();
        }
    }

    // Create stdout TX mutex
    {
        osMutexAttr_t tx_mutex_attr = {
            .name = "tx_mutex", .attr_bits = osMutexPrioInherit, .cb_mem = &tx_mutex_obj, .cb_size = sizeof(tx_mutex_obj)
        };

        tx_mutex = osMutexNew(&tx_mutex_attr);
        if (tx_mutex == NULL)
        {
            bare_metal_print("Failed to initialize tx mutex\r\n");
            abort();
        }
    }

    // Disable buffering to let write and fwrite call straight into _write
    setvbuf(stdout, /* buffer */ NULL, _IONBF, /* size */ 0);
    setvbuf(stderr, /* buffer */ NULL, _IONBF, /* size */ 0);
    setvbuf(stdin, /* buffer */ NULL, _IONBF, /* size */ 0);

    // It is safe to use printf from this point

    {
        int ret = tfm_ns_interface_init();
        if (ret != 0)
        {
            bare_metal_print("TF-M initialization failed\r\n");
            abort();
        }
    }

    /* Run the C++ global object constructors */
    __libc_init_array();

    // Note: Reception on the serial port is buffered to mitigate bytes lost
    mdh_serial_set_irq_callback(serial, serial_irq_cb, serial);
    mdh_serial_set_irq_availability(serial, MDH_SERIAL_IRQ_TYPE_RX, true);

    // It is safe to receive data on serial from this point

    int return_code = main();

    exit(return_code);
}

/*
 * Override of lock/unlock functions for malloc.
 */
extern "C" void __wrap___malloc_lock(struct _reent * reent)
{
    osMutexAcquire(malloc_mutex, osWaitForever);
}

extern "C" void __wrap___malloc_unlock(struct _reent * reent)
{
    osMutexRelease(malloc_mutex);
}

/*
 * Override of new/delete operators.
 * The override add a trace when a non-throwing new fails.
 */

void * operator new(std::size_t count)
{
    void * buffer = malloc(count);
    if (!buffer)
    {
        printf("operator new failure from %p\r\n", CALLER_ADDR());
        abort();
    }
    return buffer;
}

void * operator new[](std::size_t count)
{
    void * buffer = malloc(count);
    if (!buffer)
    {
        printf("operator new[] failure from %p\r\n", CALLER_ADDR());
        abort();
    }
    return buffer;
}

void * operator new(std::size_t count, const std::nothrow_t & tag)
{
    return malloc(count);
}

void * operator new[](std::size_t count, const std::nothrow_t & tag)
{
    return malloc(count);
}

void operator delete(void * ptr)
{
    free(ptr);
}

void operator delete(void * ptr, std::size_t)
{
    free(ptr);
}

void operator delete[](void * ptr)
{
    free(ptr);
}

void operator delete[](void * ptr, std::size_t)
{
    free(ptr);
}

/*
 * Override of _sbrk
 * It prints an error when the system runs out of memory in the heap segment.
 */

#undef errno
extern "C" int errno;

extern "C" char __end__;
extern "C" char __HeapLimit;

extern "C" void * _sbrk(int incr)
{
    static uint32_t heap = (uint32_t) &__end__;
    uint32_t prev_heap   = heap;
    uint32_t new_heap    = heap + incr;

    /* __HeapLimit is end of heap section */
    if (new_heap > (uint32_t) &__HeapLimit)
    {
        printf("_sbrk failure, incr = %d, new_heap = 0x%08lX\r\n", incr, new_heap);
        errno = ENOMEM;
        return (void *) -1;
    }

    heap = new_heap;
    return (void *) prev_heap;
}

// Override exit
extern "C" void _exit(int return_code)
{
    // display exit reason
    if (return_code)
    {
        printf("Application exited with %d\r\n", return_code);
    }

    // flush stdio
    fflush(stdout);
    fflush(stderr);

    // lock the kernel and go to sleep forever
    osKernelLock();
    while (1)
    {
        __WFE();
    }
}

// Calling a FreeRTOS API is illegal while scheduler is suspended.
// Therefore we provide this custom implementation which relies on underlying
// safety of malloc.
extern "C" void * pvPortMalloc(size_t size)
{
    return malloc(size);
}

extern "C" void vPortFree(void * ptr)
{
    free(ptr);
}

// Retarget of low level read and write

extern "C" int _write(int fd, const char * str, size_t len)
{
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO)
    {
        return -1;
    }

    osMutexAcquire(tx_mutex, osWaitForever);
    int len_written = serial_out(str, len);
    osMutexRelease(tx_mutex);

    return len_written;
}

extern "C" int _read(int fd, char * str, size_t len)
{
    if (fd != STDIN_FILENO)
    {
        return -1;
    }

    return rx_buffer.get((uint8_t *) str, len);
}

static void serial_irq_cb(void * p_instance, mdh_serial_irq_type_t event)
{
    if ((event != MDH_SERIAL_IRQ_TYPE_RX) || !p_instance)
    {
        return;
    }

    mdh_serial_t * serial = reinterpret_cast<mdh_serial_t *>(p_instance);

    if (event == MDH_SERIAL_IRQ_TYPE_RX)
    {
        while (mdh_serial_is_readable(serial))
        {
            // Gather as much as possible data bytes
            rx_buffer.put(mdh_serial_get_data(serial));
        }
    }
}

// Ring buffer implementation
template <size_t Size>
void RingBuffer<Size>::put(uint8_t data)
{
    CriticalSection _;
    buffer_[head_] = data;
    increment(head_);

    if (full_)
    {
        tail_ = head_;
    }
    else if (head_ == tail_)
    {
        full_ = true;
    }
}

template <size_t Size>
bool RingBuffer<Size>::get(uint8_t * data)
{
    CriticalSection _;
    if (empty_())
    {
        return false;
    }

    *data = buffer_[tail_];
    increment(tail_);
    full_ = false;

    return true;
}

template <size_t Size>
size_t RingBuffer<Size>::get(uint8_t * data, size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    if (size == 1)
    {
        return get(data) ? 1 : 0;
    }

    CriticalSection _;
    if (empty_())
    {
        return 0;
    }

    // resize dest to fit with available data
    size = std::min(size_(), size);

    if (tail_ + size > Size)
    {
        auto it = std::copy(buffer_ + tail_, buffer_ + Size, data);
        // set to the future tail
        tail_ = size - (Size - tail_);
        std::copy(buffer_, buffer_ + tail_, it);
    }
    else
    {
        std::copy(buffer_ + tail_, buffer_ + size, data);
        increment(tail_, size);
    }

    full_ = false;

    return size;
}

template <size_t Size>
bool RingBuffer<Size>::empty() const
{
    CriticalSection _;
    return empty_();
}

template <size_t Size>
bool RingBuffer<Size>::full() const
{
    CriticalSection _;
    return bool(full_);
}

template <size_t Size>
size_t RingBuffer<Size>::size() const
{
    CriticalSection _;
    return size_();
}

template <size_t Size>
void RingBuffer<Size>::increment(size_t & val)
{
    ++val;
    assert(val <= Size);

    if (val == Size)
    {
        val = 0;
    }
}

template <size_t Size>
void RingBuffer<Size>::increment(size_t & val, size_t incr)
{
    val += incr;

    if (val >= Size)
    {
        val = val - Size;
    }
}

template <size_t Size>
bool RingBuffer<Size>::empty_() const
{
    return head_ == tail_ && !bool(full_);
}

template <size_t Size>
size_t RingBuffer<Size>::size_() const
{
    if (full_)
    {
        return Size;
    }
    else if (head_ < tail_)
    {
        return Size - (tail_ - head_);
    }
    else
    {
        return head_ - tail_;
    };
}
