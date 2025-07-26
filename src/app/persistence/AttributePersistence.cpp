/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <app/persistence/AttributePersistence.h>

namespace chip::app {

namespace {

bool VerifySuccessLogOnFailure(const ConcreteAttributePath & path, CHIP_ERROR err)
{
    VerifyOrReturnValue(err != CHIP_NO_ERROR, true);

    // assume value not found being typical.
    VerifyOrReturnValue(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, false);

    ChipLogError(Zcl, "Failed to load attribute %u/" ChipLogFormatMEI "/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                 path.mEndpointId, ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId), err.Format());
    return false;
}

} // namespace

bool AttributePersistence::Load(const ConcreteAttributePath & path, Storage::ShortPascalString & value,
                                std::optional<CharSpan> valueOnLoadFailure)
{
    MutableByteSpan rawBytes = value.RawFullBuffer();

    if (!VerifySuccessLogOnFailure(path, mProvider.ReadValue(path, rawBytes)))
    {
        if (valueOnLoadFailure.has_value() && value.SetValue(*valueOnLoadFailure))
        {
            // have a default that could be set
            return false;
        }
        // no default value or default set failed. Set null
        value.SetNull();
        return false;
    }
    return true;
}

DataModel::ActionReturnStatus AttributePersistence::Store(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                                          Storage::ShortPascalString & value)
{
    CharSpan spanValue;
    ReturnErrorOnFailure(decoder.Decode(spanValue));
    VerifyOrReturnError(value.SetValue(spanValue), Protocols::InteractionModel::Status::ConstraintError);
    return mProvider.WriteValue(path, value.ContentWithLenPrefix());
}

bool AttributePersistence::InternalRawLoadNativeEndianValue(const ConcreteAttributePath & path, void * data,
                                                            const void * valueOnLoadFailure, size_t size)
{
    MutableByteSpan rawBytes(reinterpret_cast<uint8_t *>(data), size);
    if (!VerifySuccessLogOnFailure(path, mProvider.ReadValue(path, rawBytes)))
    {
        /// in case of failure, set the default value
        memcpy(data, valueOnLoadFailure, size);
        return false;
    }
    return true;
}

} // namespace chip::app
