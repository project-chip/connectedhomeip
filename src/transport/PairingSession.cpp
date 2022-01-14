/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <transport/PairingSession.h>

#include <lib/core/CHIPTLVTypes.h>
#include <lib/support/SafeInt.h>

namespace chip {

CHIP_ERROR PairingSession::EncodeMRPParameters(TLV::Tag tag, const ReliableMessageProtocolConfig & mrpConfig,
                                               TLV::TLVWriter & tlvWriter)
{
    VerifyOrReturnError(CanCastTo<uint16_t>(mrpConfig.mIdleRetransTimeout.count()), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(mrpConfig.mActiveRetransTimeout.count()), CHIP_ERROR_INVALID_ARGUMENT);

    TLV::TLVType mrpParamsContainer;
    ReturnErrorOnFailure(tlvWriter.StartContainer(tag, TLV::kTLVType_Structure, mrpParamsContainer));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), static_cast<uint16_t>(mrpConfig.mIdleRetransTimeout.count())));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), static_cast<uint16_t>(mrpConfig.mActiveRetransTimeout.count())));
    return tlvWriter.EndContainer(mrpParamsContainer);
}

CHIP_ERROR PairingSession::DecodeMRPParametersIfPresent(TLV::Tag expectedTag, TLV::ContiguousBufferTLVReader & tlvReader)
{
    // The MRP parameters are optional.
    if (tlvReader.GetTag() != expectedTag)
    {
        return CHIP_NO_ERROR;
    }

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    uint16_t tlvElementValue = 0;

    ReturnErrorOnFailure(tlvReader.Next());

    ChipLogDetail(SecureChannel, "Found MRP parameters in the message");

    // Both TLV elements in the structure are optional. If the first element is present, process it and move
    // the TLV reader to the next element.
    if (TLV::TagNumFromTag(tlvReader.GetTag()) == 1)
    {
        ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
        mMRPConfig.mIdleRetransTimeout = System::Clock::Milliseconds32(tlvElementValue);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        CHIP_ERROR err = tlvReader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return tlvReader.ExitContainer(containerType);
        }
        ReturnErrorOnFailure(err);
    }

    VerifyOrReturnError(TLV::TagNumFromTag(tlvReader.GetTag()) == 2, CHIP_ERROR_INVALID_TLV_TAG);
    ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
    mMRPConfig.mActiveRetransTimeout = System::Clock::Milliseconds32(tlvElementValue);

    return tlvReader.ExitContainer(containerType);
}

} // namespace chip
