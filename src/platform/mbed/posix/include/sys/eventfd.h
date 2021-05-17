/*
 * Copyright (c) 2020 Tobias Svehagen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_POSIX_SYS_EVENTFD_H
#define MBED_POSIX_SYS_EVENTFD_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EFD_IN_USE 0x1
#define EFD_SEMAPHORE 0x2
#define EFD_NONBLOCK O_NONBLOCK
#define EFD_FLAGS_SET (EFD_SEMAPHORE | EFD_NONBLOCK)

typedef uint64_t eventfd_t;

/**
 * @brief Create a file descriptor for event notification
 *
 * The returned file descriptor can be used with POSIX read/write calls or
 * with the eventfd_read/eventfd_write functions.
 *
 * It also supports polling and by including an eventfd in a call to poll,
 * it is possible to signal and wake the polling thread by simply writing to
 * the eventfd.
 *
 * When using read() and write() on an eventfd, the size must always be at
 * least 8 bytes or the operation will fail with EINVAL.
 *
 * @return New eventfd file descriptor on success, -1 on error
 */
int eventfd(unsigned int initval, int flags);
/**
 * @brief Read from an eventfd
 *
 * If call is successful, the value parameter will have the value 1
 *
 * @param fd File descriptor
 * @param value Pointer for storing the read value
 *
 * @return 0 on success, -1 on error
 */
int eventfd_read(int fd, eventfd_t * value);
/**
 * @brief Write to an eventfd
 *
 * @param fd File descriptor
 * @param value Value to write
 *
 * @return 0 on success, -1 on error
 */
int eventfd_write(int fd, eventfd_t value);

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_SYS_EVENTFD_H */
