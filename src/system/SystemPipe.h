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
 *      This file declares the abstraction of one-directional, anonymous
 *      data stream built on top of two file descriptors.
 */

#ifndef SYSTEMPIPE_H
#define SYSTEMPIPE_H

// Include configuration headers
#include <system/SystemConfig.h>

#include <system/SystemError.h>

namespace chip {
namespace System {

using ::chip::System::Error;

class Pipe
{
public:
    Error Open(); /**< Initialize the pipeline */
    void Close(); /**< Close both ends of the pipeline. */

    int GetReadFD() const { return mFDs[FD_READ]; }

    void WriteByte(uint8_t byte); /**< Put a single byte in the pipeline */
    void ClearContent();          /**< Clear the pipeline by reading all its contents */

private:
    enum
    {
        FD_READ  = 0,
        FD_WRITE = 1
    };

    int mFDs[2];
};

} // namespace System
} // namespace chip

#endif // SYSTEMPIPE_H