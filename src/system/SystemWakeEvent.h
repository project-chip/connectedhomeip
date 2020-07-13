/*
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
 *      This file declares the abstraction of system wake event used for
 *      resuming task from select system call.
 */

#ifndef SYSTEMWAKEEVENT_H
#define SYSTEMWAKEEVENT_H

// Include configuration headers
#include <system/SystemConfig.h>

#include <system/SystemError.h>

namespace chip {
namespace System {

using ::chip::System::Error;

class SystemWakeEvent
{
public:
    Error Open(); /**< Initialize the pipeline */
    void Close(); /**< Close both ends of the pipeline. */

#if CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
    int GetNotifFD() const { return mFDs[FD_READ]; }
#else
    int GetNotifFD() const { return mFD; }
#endif

    void Notify();  /**< Set the event. */
    void Confirm(); /**< Clear the event. */

private:
#if CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
    enum
    {
        FD_READ  = 0,
        FD_WRITE = 1
    };

    int mFDs[2];
#else
    int mFD;
#endif
};

} // namespace System
} // namespace chip

#endif // SYSTEMWAKEEVENT_H
