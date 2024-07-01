/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include "ProvisionEncoder.h"
#include "ProvisionStorage.h"
#include <lib/support/Span.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

//------------------------------------------------------------------------------
// Common
//------------------------------------------------------------------------------

class Protocol
{
public:
    /**
     * Must hold the header plus complete argument value
     */
    Protocol(Storage & store) : mStore(store) {}
    virtual ~Protocol()                                                  = default;
    virtual bool Execute(ByteSpan & request, MutableByteSpan & response) = 0;

protected:
    Storage & mStore;
};

//------------------------------------------------------------------------------
// Version 1
//------------------------------------------------------------------------------
#ifdef SILABS_PROVISION_PROTOCOL_V1

class Protocol1 : public Protocol
{
public:
    static constexpr size_t kVersion = 1;

    Protocol1(Storage & store) : Protocol(store) {}
    virtual bool Execute(ByteSpan & request, MutableByteSpan & response);

private:
    CHIP_ERROR Init(Encoding::Buffer & in, Encoding::Buffer & out);
    CHIP_ERROR GenerateCSR(Encoding::Buffer & in, Encoding::Buffer & out);
    CHIP_ERROR Import(Encoding::Buffer & in, Encoding::Buffer & out);
    CHIP_ERROR Setup(Encoding::Buffer & in, Encoding::Buffer & out);
    CHIP_ERROR Read(Encoding::Buffer & in, Encoding::Buffer & out);
};

#endif // SILABS_PROVISION_PROTOCOL_V1

//------------------------------------------------------------------------------
// Version 2
//------------------------------------------------------------------------------

class Protocol2 : public Protocol
{
public:
    static constexpr uint8_t kVersion = 2;
    // Command flags
    static constexpr uint16_t kResponseFlag = 0x80;
    // Payload flags
    static constexpr uint16_t kCustomIdMin   = 0x0000;
    static constexpr uint16_t kCustomIdMax   = 0x00ff;
    static constexpr uint16_t kKnownIdMin    = 0x0100;
    static constexpr uint16_t kKnownIdMax    = 0x01ff;
    static constexpr uint16_t kIdMask        = 0x01ff;
    static constexpr uint16_t kWellKnownMask = 0x0100;
    static constexpr uint16_t kSizeBit       = 10;
    static constexpr uint16_t kSizeMask      = 0x0c00;
    static constexpr uint16_t kFeedbackMask  = 0x0200;
    static constexpr uint16_t kTypeBit       = 12;
    static constexpr uint16_t kTypeMask      = 0xf000;
    // Limits
    static constexpr size_t kPackageSizeMax     = 128;
    static constexpr size_t kChecksumSize       = 2;
    static constexpr size_t kRequestHeaderSize  = 4;
    static constexpr size_t kResponseHeaderSize = 8;
    static_assert(kPackageSizeMax > (kResponseHeaderSize + kChecksumSize));

    Protocol2(Storage & store) : Protocol(store) {}
    virtual bool Execute(ByteSpan & request, MutableByteSpan & response);
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
