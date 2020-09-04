/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements methods for the Nest Weave Status Report
 *      profile.
 */

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <protocols/CHIPProtocols.h>
#include <protocols/common/CommonProtocol.h>

#include "StatusReportProtocol.h"

using namespace chip;
using namespace chip::TLV;
using namespace chip::Protocols;
using namespace chip::Protocols::Common;

namespace chip {
namespace Protocols {
namespace StatusReporting {

StatusReport::StatusReport(void)
{
    mProtocolId = 0;
    mStatusCode = 0;
    mError = CHIP_NO_ERROR;
}

StatusReport::~StatusReport(void)
{
    mProtocolId = 0;
    mStatusCode = 0;
    mError = CHIP_NO_ERROR;
}

CHIP_ERROR StatusReport::init(uint32_t aProfileId, uint16_t aCode, ReferencedTLVData *aInfo)
{
    mProtocolId = aProfileId;
    mStatusCode = aCode;
    mError = CHIP_NO_ERROR;

    if (aInfo)
        mAdditionalInfo = *aInfo;

    return CHIP_NO_ERROR;
}

CHIP_ERROR StatusReport::init(CHIP_ERROR aError)
{
    if (aError == CHIP_NO_ERROR)
    {
        mProtocolId = kChipProtocol_Common;
        mStatusCode = kStatus_Success;
    }

    else
    {
        mProtocolId = kChipProtocol_Common;
        mStatusCode = kStatus_InternalError;
        mError = aError;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR StatusReport::pack(PacketBuffer *aBuffer, uint32_t maxLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageIterator i(aBuffer);
    TLVWriter writer;

    i.append();

    err = i.write32(mProtocolId);
    SuccessOrExit(err);

    err = i.write16(mStatusCode);
    SuccessOrExit(err);

    /*
     * the assumption here is that EITHER there's
     * an error code that wants to be included as
     * metadata OR there's additional info passed
     * in at initialization time, which may include
     * an error, OR there's none of the above, in
     * which case the else clause her writes nothing
     */

    if (mError != CHIP_NO_ERROR)
    {
        writer.Init(aBuffer);

        err = StartMetaData(writer);
        SuccessOrExit(err);

        err = AddErrorCode(writer, mError);
        SuccessOrExit(err);

        err = EndMetaData(writer);
        SuccessOrExit(err);

        /*
         * this is a bit of a hack. we basically set this
         * so that packedLength() below will return the right
         * number at least if we call it AFTER the thing has
         * been packed. it's not clear we ever use this.
         */

        mAdditionalInfo.theLength = writer.GetLengthWritten();
    }

    else
    {
        err = mAdditionalInfo.pack(i, maxLen - 6);
    }

exit:
    return err;
}

inline uint16_t StatusReport::packedLength(void)
{
    return sizeof(mProtocolId)+sizeof(mStatusCode)+mAdditionalInfo.theLength;
}

CHIP_ERROR StatusReport::parse(PacketBuffer *aBuffer, StatusReport &aDestination)
{
    CHIP_ERROR err;
    MessageIterator i(aBuffer);

    err = i.read32(&aDestination.mProtocolId);
    SuccessOrExit(err);

    err = i.read16(&aDestination.mStatusCode);
    SuccessOrExit(err);

    err = ReferencedTLVData::parse(i, aDestination.mAdditionalInfo);

exit:
    return err;
}

bool StatusReport::operator == (const StatusReport &another) const
{
    return ((mProtocolId == another.mProtocolId) && (mStatusCode == another.mStatusCode));
}

/*
 * the universal, gold standard for success is
 * <Nest Labs> : <Common Profile> : <Success>
 */

bool StatusReport::success(void)
{
    return(mProtocolId == kChipProtocol_Common && mStatusCode == kStatus_Success);
}

CHIP_ERROR StatusReport::StartMetaData(chip::TLV::TLVWriter & aWriter)
{
    TLVType metaDataContainer;

    return aWriter.StartContainer(AnonymousTag, kTLVType_Structure, metaDataContainer);
}

CHIP_ERROR StatusReport::EndMetaData(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err;
    TLVType metaDataContainer = kTLVType_Structure;

    err = aWriter.EndContainer(metaDataContainer);
    SuccessOrExit(err);

    err = aWriter.Finalize();

exit:
    return err;
}

CHIP_ERROR StatusReport::AddErrorCode(chip::TLV::TLVWriter & aWriter, CHIP_ERROR aError)
{
    return aWriter.Put(CommonTag(kTag_SystemErrorCode), aError);
}

} // namespace StatusReporting
} // namespace Protocols
} // namespace chip
