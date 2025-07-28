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
#include "app/ConcreteAttributePath.h"
#include "app/data-model/Nullable.h"
#include "app/persistence/AttributePersistenceProvider.h"
#include "app/persistence/PascalString.h"
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

void LogInvalidPascalContent(const ConcreteAttributePath & path)
{
    ChipLogError(Zcl, "Invalid pascal content: %u/" ChipLogFormatMEI "/" ChipLogFormatMEI, path.mEndpointId,
                 ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
}

template <typename StringType>
bool LoadPascalStringImpl(AttributePersistenceProvider & provider, const ConcreteAttributePath & path, StringType & value,
                          std::optional<typename StringType::ValueType> valueOnLoadFailure)
{
    MutableByteSpan rawBytes = value.RawFullBuffer();

    if (VerifySuccessLogOnFailure(path, provider.ReadValue(path, rawBytes)))
    {
        // value is read, however it has to be a valid pascal value
        // The value MUST be valid over the read range (i.e. rawBytes)
        auto concrete = reinterpret_cast<const typename StringType::ValueType::pointer>(rawBytes.data());
        if (value.IsValid({ concrete, rawBytes.size() }))
        {
            return true;
        }

        LogInvalidPascalContent(path);
    }

    // failed, try to set some default
    if (!valueOnLoadFailure.has_value() || !value.SetValue(*valueOnLoadFailure))
    {
        value.SetNull();
    }
    return false;
}

} // namespace

bool AttributePersistence::Load(const ConcreteAttributePath & path, Storage::ShortPascalString & value,
                                std::optional<CharSpan> valueOnLoadFailure)
{
    return LoadPascalStringImpl(mProvider, path, value, valueOnLoadFailure);
}

bool AttributePersistence::Load(const ConcreteAttributePath & path, Storage::ShortPascalBytes & value,
                                std::optional<ByteSpan> valueOnLoadFailure)
{
    return LoadPascalStringImpl(mProvider, path, value, valueOnLoadFailure);
}

DataModel::ActionReturnStatus AttributePersistence::Store(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                                          Storage::ShortPascalString & value)
{
    DataModel::Nullable<CharSpan> spanValue;
    ReturnErrorOnFailure(decoder.Decode(spanValue));
    if (spanValue.IsNull())
    {
        value.SetNull();
    }
    else
    {
        VerifyOrReturnError(value.SetValue(spanValue.Value()), Protocols::InteractionModel::Status::ConstraintError);
    }
    return mProvider.WriteValue(path, value.ContentWithLenPrefix());
}

DataModel::ActionReturnStatus AttributePersistence::Store(const ConcreteAttributePath & path, AttributeValueDecoder & decoder,
                                                          Storage::ShortPascalBytes & value)
{
    DataModel::Nullable<ByteSpan> spanValue;
    ReturnErrorOnFailure(decoder.Decode(spanValue));
    if (spanValue.IsNull())
    {
        value.SetNull();
    }
    else
    {
        VerifyOrReturnError(value.SetValue(spanValue.Value()), Protocols::InteractionModel::Status::ConstraintError);
    }
    return mProvider.WriteValue(path, value.ContentWithLenPrefix());
}

bool AttributePersistence::InternalRawLoadNativeEndianValue(const ConcreteAttributePath & path, void * data,
                                                            const void * valueOnLoadFailure, size_t size)
{
    MutableByteSpan rawBytes(reinterpret_cast<uint8_t *>(data), size);
    if (!VerifySuccessLogOnFailure(path, mProvider.ReadValue(path, rawBytes)))
    {
        // in case of failure, set the default value
        memcpy(data, valueOnLoadFailure, size);
        return false;
    }

    if (rawBytes.size() != size)
    {
        // short read: the value is not valid
        memcpy(data, valueOnLoadFailure, size);
        return false;
    }

    return true;
}

} // namespace chip::app
