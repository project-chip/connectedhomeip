#/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This header file defines BSD socket API functions which for various
 *      reasons have not (yet) been implemented or exposed in Zephyr.
 */

#pragma once

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/select.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <zephyr/net/socket.h>
#endif

static inline ssize_t recvmsg(int sock, struct msghdr * msg, int flags)
{
    // Zephyr doesn't implement recvmsg at all, but if the message vector size is > 0 we can simply
    // translate recvmsg to recvfrom which fills only the first of the provided buffers (although
    // we don't get control messages in such a case).

    if (msg->msg_iovlen < 1)
    {
        errno = EMSGSIZE;
        return -1;
    }

    ssize_t ret = recvfrom(sock, msg->msg_iov[0].iov_base, msg->msg_iov[0].iov_len, flags, static_cast<sockaddr *>(msg->msg_name),
                           &msg->msg_namelen);

    if (ret >= 0)
        msg->msg_controllen = 0;

    return ret;
}
