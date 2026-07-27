/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/server/ThreadRendezvousAnnouncement.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <lib/support/CodeUtils.h>

#include <lib/dnssd/minimal_mdns/ResponseBuilder.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>

namespace chip {
namespace app {

CHIP_ERROR TxtStringsBuilder::Fill(const Dnssd::CommissionAdvertisingParameters & params)
{
    // VP
    auto vendorId  = params.GetVendorId();
    auto productId = params.GetProductId();
    if (vendorId.has_value() && productId.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("VP=%d+%d", *vendorId, *productId));
    }
    else if (vendorId.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("VP=%d", *vendorId));
    }

    // D
    ReturnErrorOnFailure(FormatAndAdd("D=%d", params.GetLongDiscriminator()));

    // CM
    ReturnErrorOnFailure(FormatAndAdd("CM=%d", static_cast<int>(params.GetCommissioningMode())));

    // DT
    auto deviceType = params.GetDeviceType();
    if (deviceType.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("DT=%" PRIu32, *deviceType));
    }

    // DN
    auto deviceName = params.GetDeviceName();
    if (deviceName.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("DN=%s", *deviceName));
    }

    // RI
    auto rotatingId = params.GetRotatingDeviceId();
    if (rotatingId.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("RI=%s", *rotatingId));
    }

    // PH
    auto pairingHint = params.GetPairingHint();
    if (pairingHint.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("PH=%d", *pairingHint));
    }

    // PI
    auto pairingInstr = params.GetPairingInstruction();
    if (pairingInstr.has_value())
    {
        ReturnErrorOnFailure(FormatAndAdd("PI=%s", *pairingInstr));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TxtStringsBuilder::FormatAndAdd(const char * format, ...)
{
    VerifyOrReturnError(mCount < MATTER_ARRAY_SIZE(mTxtStrings), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t offset = static_cast<size_t>(mNextStart - &mTxtBuffer[0]);
    VerifyOrReturnError(sizeof(mTxtBuffer) > offset, CHIP_ERROR_BUFFER_TOO_SMALL);

    va_list args;
    va_start(args, format);
    int len = vsnprintf(mNextStart, sizeof(mTxtBuffer) - offset, format, args);
    va_end(args);

    // Make sure not overflowed.
    VerifyOrReturnError(len > 0 && offset + static_cast<size_t>(len) < sizeof(mTxtBuffer), CHIP_ERROR_BUFFER_TOO_SMALL);

    mTxtStrings[mCount++] = mNextStart;

    // skip the null terminator.
    mNextStart += len + 1;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BuildThreadRendezvousAnnouncement(const Dnssd::CommissionAdvertisingParameters & params,
                                             System::PacketBufferHandle & outBuffer)
{
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    {
        mdns::Minimal::ResponseBuilder builder(std::move(buffer));
        builder.Header().SetMessageId(0);
        builder.Header().SetFlags(builder.Header().GetFlags().SetResponse().SetAuthoritative());

        static const char * matterc_udp_local[] = { "_matterc", "_udp", "local" };
        mdns::Minimal::FullQName serviceName(matterc_udp_local);

        static const char * root[] = { "" };
        mdns::Minimal::FullQName targetName(root);
        mdns::Minimal::SrvResourceRecord srvRecord(serviceName, targetName, params.GetPort());
        builder.AddRecord(mdns::Minimal::ResourceType::kAnswer, srvRecord);

        TxtStringsBuilder txtStringsBuilder;
        ReturnErrorOnFailure(txtStringsBuilder.Fill(params));
        mdns::Minimal::TxtResourceRecord txtRecord(serviceName, txtStringsBuilder.GetEntries(), txtStringsBuilder.GetCount());
        builder.AddRecord(mdns::Minimal::ResourceType::kAnswer, txtRecord);

        outBuffer = builder.ReleasePacket();
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
