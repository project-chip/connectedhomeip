#/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
