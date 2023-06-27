/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRCallbackBridge.h"
#import "MTRCommandPayloadsObjc.h"
#import "MTRCommandPayloads_Internal.h"
#import "MTRStructsObjc.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <lib/support/TypeTraits.h>

void MTRDefaultSuccessCallbackBridge::OnSuccessFn(void * context) { DispatchSuccess(context, nil); };

void MTRCommandSuccessCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::NullObjectType &)
{
    DispatchSuccess(context, nil);
};

void MTROctetStringAttributeCallbackBridge::OnSuccessFn(void * context, chip::ByteSpan value)
{
    NSData * _Nonnull objCValue;
    objCValue = AsData(value);
    DispatchSuccess(context, objCValue);
};

void MTROctetStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOctetStringAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::ByteSpan> & value)
{
    NSData * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = AsData(value.Value());
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOctetStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCharStringAttributeCallbackBridge::OnSuccessFn(void * context, chip::CharSpan value)
{
    NSString * _Nonnull objCValue;
    objCValue = AsString(value);
    if (objCValue == nil) {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCharStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCharStringAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::CharSpan> & value)
{
    NSString * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = AsString(value.Value());
        if (objCValue == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCharStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBooleanAttributeCallbackBridge::OnSuccessFn(void * context, bool value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithBool:value];
    DispatchSuccess(context, objCValue);
};

void MTRBooleanAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableBooleanAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<bool> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithBool:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableBooleanAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt8uAttributeCallbackBridge::OnSuccessFn(void * context, uint8_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt8uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt8uAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<uint8_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt8uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt8sAttributeCallbackBridge::OnSuccessFn(void * context, int8_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithChar:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt8sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt8sAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<int8_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithChar:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt8sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt16uAttributeCallbackBridge::OnSuccessFn(void * context, uint16_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt16uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt16uAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<uint16_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt16uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt16sAttributeCallbackBridge::OnSuccessFn(void * context, int16_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithShort:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt16sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt16sAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<int16_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithShort:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt16sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt32uAttributeCallbackBridge::OnSuccessFn(void * context, uint32_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt32uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt32uAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<uint32_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedInt:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt32uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt32sAttributeCallbackBridge::OnSuccessFn(void * context, int32_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithInt:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt32sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt32sAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<int32_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithInt:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt32sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt64uAttributeCallbackBridge::OnSuccessFn(void * context, uint64_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedLongLong:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt64uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt64uAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<uint64_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedLongLong:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt64uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRInt64sAttributeCallbackBridge::OnSuccessFn(void * context, int64_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithLongLong:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt64sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableInt64sAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<int64_t> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithLongLong:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableInt64sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFloatAttributeCallbackBridge::OnSuccessFn(void * context, float value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithFloat:value];
    DispatchSuccess(context, objCValue);
};

void MTRFloatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFloatAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<float> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithFloat:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFloatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoubleAttributeCallbackBridge::OnSuccessFn(void * context, double value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithDouble:value];
    DispatchSuccess(context, objCValue);
};

void MTRDoubleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoubleAttributeCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::Nullable<double> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithDouble:value.Value()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoubleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRVendorIdAttributeCallbackBridge::OnSuccessFn(void * context, chip::VendorId value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRVendorIdAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableVendorIdAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::VendorId> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableVendorIdAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsNameSupportAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::Groups::NameSupportBitmap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRGroupsNameSupportAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRScenesGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRScenesGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRScenesAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRScenesAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRScenesEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRScenesEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRScenesAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRScenesAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffSwitchConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffSwitchConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffSwitchConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROnOffSwitchConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlOptionsAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlOptionsAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBinaryInputBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBinaryInputBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBinaryInputBasicEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBinaryInputBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPulseWidthModulationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPulseWidthModulationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorDeviceTypeListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRDescriptorClusterDeviceTypeStruct * newElement_0;
            newElement_0 = [MTRDescriptorClusterDeviceTypeStruct new];
            newElement_0.deviceType = [NSNumber numberWithUnsignedInt:entry_0.deviceType];
            newElement_0.revision = [NSNumber numberWithUnsignedShort:entry_0.revision];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorDeviceTypeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorServerListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::ClusterId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorServerListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorClientListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::ClusterId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorClientListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorPartsListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EndpointId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorPartsListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDescriptorAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDescriptorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBindingBindingListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Binding::Structs::TargetStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRBindingClusterTargetStruct * newElement_0;
            newElement_0 = [MTRBindingClusterTargetStruct new];
            if (entry_0.node.HasValue()) {
                newElement_0.node = [NSNumber numberWithUnsignedLongLong:entry_0.node.Value()];
            } else {
                newElement_0.node = nil;
            }
            if (entry_0.group.HasValue()) {
                newElement_0.group = [NSNumber numberWithUnsignedShort:entry_0.group.Value()];
            } else {
                newElement_0.group = nil;
            }
            if (entry_0.endpoint.HasValue()) {
                newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint.Value()];
            } else {
                newElement_0.endpoint = nil;
            }
            if (entry_0.cluster.HasValue()) {
                newElement_0.cluster = [NSNumber numberWithUnsignedInt:entry_0.cluster.Value()];
            } else {
                newElement_0.cluster = nil;
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBindingBindingListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBindingGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBindingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBindingAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBindingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBindingEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBindingEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBindingAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBindingAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlACLListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAccessControlClusterAccessControlEntryStruct * newElement_0;
            newElement_0 = [MTRAccessControlClusterAccessControlEntryStruct new];
            newElement_0.privilege = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.privilege)];
            newElement_0.authMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.authMode)];
            if (entry_0.subjects.IsNull()) {
                newElement_0.subjects = nil;
            } else {
                { // Scope for our temporary variables
                    auto * array_3 = [NSMutableArray new];
                    auto iter_3 = entry_0.subjects.Value().begin();
                    while (iter_3.Next()) {
                        auto & entry_3 = iter_3.GetValue();
                        NSNumber * newElement_3;
                        newElement_3 = [NSNumber numberWithUnsignedLongLong:entry_3];
                        [array_3 addObject:newElement_3];
                    }
                    CHIP_ERROR err = iter_3.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        OnFailureFn(context, err);
                        return;
                    }
                    newElement_0.subjects = array_3;
                }
            }
            if (entry_0.targets.IsNull()) {
                newElement_0.targets = nil;
            } else {
                { // Scope for our temporary variables
                    auto * array_3 = [NSMutableArray new];
                    auto iter_3 = entry_0.targets.Value().begin();
                    while (iter_3.Next()) {
                        auto & entry_3 = iter_3.GetValue();
                        MTRAccessControlClusterAccessControlTargetStruct * newElement_3;
                        newElement_3 = [MTRAccessControlClusterAccessControlTargetStruct new];
                        if (entry_3.cluster.IsNull()) {
                            newElement_3.cluster = nil;
                        } else {
                            newElement_3.cluster = [NSNumber numberWithUnsignedInt:entry_3.cluster.Value()];
                        }
                        if (entry_3.endpoint.IsNull()) {
                            newElement_3.endpoint = nil;
                        } else {
                            newElement_3.endpoint = [NSNumber numberWithUnsignedShort:entry_3.endpoint.Value()];
                        }
                        if (entry_3.deviceType.IsNull()) {
                            newElement_3.deviceType = nil;
                        } else {
                            newElement_3.deviceType = [NSNumber numberWithUnsignedInt:entry_3.deviceType.Value()];
                        }
                        [array_3 addObject:newElement_3];
                    }
                    CHIP_ERROR err = iter_3.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        OnFailureFn(context, err);
                        return;
                    }
                    newElement_0.targets = array_3;
                }
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlACLListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlExtensionListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::AccessControl::Structs::AccessControlExtensionStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAccessControlClusterAccessControlExtensionStruct * newElement_0;
            newElement_0 = [MTRAccessControlClusterAccessControlExtensionStruct new];
            newElement_0.data = AsData(entry_0.data);
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlExtensionListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsActionListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Actions::Structs::ActionStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRActionsClusterActionStruct * newElement_0;
            newElement_0 = [MTRActionsClusterActionStruct new];
            newElement_0.actionID = [NSNumber numberWithUnsignedShort:entry_0.actionID];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
            newElement_0.endpointListID = [NSNumber numberWithUnsignedShort:entry_0.endpointListID];
            newElement_0.supportedCommands = [NSNumber numberWithUnsignedShort:entry_0.supportedCommands.Raw()];
            newElement_0.state = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.state)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsActionListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsEndpointListsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Actions::Structs::EndpointListStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRActionsClusterEndpointListStruct * newElement_0;
            newElement_0 = [MTRActionsClusterEndpointListStruct new];
            newElement_0.endpointListID = [NSNumber numberWithUnsignedShort:entry_0.endpointListID];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.endpoints.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSNumber * newElement_2;
                    newElement_2 = [NSNumber numberWithUnsignedShort:entry_2];
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.endpoints = array_2;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsEndpointListsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActionsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationCapabilityMinimaStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::BasicInformation::Structs::CapabilityMinimaStruct::DecodableType & value)
{
    MTRBasicInformationClusterCapabilityMinimaStruct * _Nonnull objCValue;
    objCValue = [MTRBasicInformationClusterCapabilityMinimaStruct new];
    objCValue.caseSessionsPerFabric = [NSNumber numberWithUnsignedShort:value.caseSessionsPerFabric];
    objCValue.subscriptionsPerFabric = [NSNumber numberWithUnsignedShort:value.subscriptionsPerFabric];
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationCapabilityMinimaStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationProductAppearanceStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::BasicInformation::Structs::ProductAppearanceStruct::DecodableType & value)
{
    MTRBasicInformationClusterProductAppearanceStruct * _Nonnull objCValue;
    objCValue = [MTRBasicInformationClusterProductAppearanceStruct new];
    objCValue.finish = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.finish)];
    if (value.primaryColor.IsNull()) {
        objCValue.primaryColor = nil;
    } else {
        objCValue.primaryColor = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.primaryColor.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationProductAppearanceStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorDefaultOTAProvidersListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROTASoftwareUpdateRequestorClusterProviderLocation * newElement_0;
            newElement_0 = [MTROTASoftwareUpdateRequestorClusterProviderLocation new];
            newElement_0.providerNodeID = [NSNumber numberWithUnsignedLongLong:entry_0.providerNodeID];
            newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint];
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorDefaultOTAProvidersListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationSupportedLocalesListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CharSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSString * newElement_0;
            newElement_0 = AsString(entry_0);
            if (newElement_0 == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLocalizationConfigurationSupportedLocalesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLocalizationConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLocalizationConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLocalizationConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLocalizationConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationSourcesListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<uint8_t> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceConfigurationSourcesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveWiredFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::WiredFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceActiveWiredFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveBatFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::BatFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceActiveBatFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveBatChargeFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::BatChargeFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceActiveBatChargeFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningBasicCommissioningInfoStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Structs::BasicCommissioningInfo::DecodableType & value)
{
    MTRGeneralCommissioningClusterBasicCommissioningInfo * _Nonnull objCValue;
    objCValue = [MTRGeneralCommissioningClusterBasicCommissioningInfo new];
    objCValue.failSafeExpiryLengthSeconds = [NSNumber numberWithUnsignedShort:value.failSafeExpiryLengthSeconds];
    objCValue.maxCumulativeFailsafeSeconds = [NSNumber numberWithUnsignedShort:value.maxCumulativeFailsafeSeconds];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningBasicCommissioningInfoStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningNetworksListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::NetworkCommissioning::Structs::NetworkInfoStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRNetworkCommissioningClusterNetworkInfoStruct * newElement_0;
            newElement_0 = [MTRNetworkCommissioningClusterNetworkInfoStruct new];
            newElement_0.networkID = AsData(entry_0.networkID);
            newElement_0.connected = [NSNumber numberWithBool:entry_0.connected];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningNetworksListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::GeneralDiagnostics::Structs::NetworkInterface::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRGeneralDiagnosticsClusterNetworkInterface * newElement_0;
            newElement_0 = [MTRGeneralDiagnosticsClusterNetworkInterface new];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.isOperational = [NSNumber numberWithBool:entry_0.isOperational];
            if (entry_0.offPremiseServicesReachableIPv4.IsNull()) {
                newElement_0.offPremiseServicesReachableIPv4 = nil;
            } else {
                newElement_0.offPremiseServicesReachableIPv4 =
                    [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv4.Value()];
            }
            if (entry_0.offPremiseServicesReachableIPv6.IsNull()) {
                newElement_0.offPremiseServicesReachableIPv6 = nil;
            } else {
                newElement_0.offPremiseServicesReachableIPv6 =
                    [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv6.Value()];
            }
            newElement_0.hardwareAddress = AsData(entry_0.hardwareAddress);
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.IPv4Addresses.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSData * newElement_2;
                    newElement_2 = AsData(entry_2);
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.iPv4Addresses = array_2;
            }
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.IPv6Addresses.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSData * newElement_2;
                    newElement_2 = AsData(entry_2);
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.iPv6Addresses = array_2;
            }
            newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::GeneralDiagnostics::HardwareFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::GeneralDiagnostics::RadioFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::GeneralDiagnostics::NetworkFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::SoftwareDiagnostics::Structs::ThreadMetricsStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRSoftwareDiagnosticsClusterThreadMetricsStruct * newElement_0;
            newElement_0 = [MTRSoftwareDiagnosticsClusterThreadMetricsStruct new];
            newElement_0.id = [NSNumber numberWithUnsignedLongLong:entry_0.id];
            if (entry_0.name.HasValue()) {
                newElement_0.name = AsString(entry_0.name.Value());
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.name = nil;
            }
            if (entry_0.stackFreeCurrent.HasValue()) {
                newElement_0.stackFreeCurrent = [NSNumber numberWithUnsignedInt:entry_0.stackFreeCurrent.Value()];
            } else {
                newElement_0.stackFreeCurrent = nil;
            }
            if (entry_0.stackFreeMinimum.HasValue()) {
                newElement_0.stackFreeMinimum = [NSNumber numberWithUnsignedInt:entry_0.stackFreeMinimum.Value()];
            } else {
                newElement_0.stackFreeMinimum = nil;
            }
            if (entry_0.stackSize.HasValue()) {
                newElement_0.stackSize = [NSNumber numberWithUnsignedInt:entry_0.stackSize.Value()];
            } else {
                newElement_0.stackSize = nil;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSoftwareDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSoftwareDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSoftwareDiagnosticsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSoftwareDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsNeighborTableListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::ThreadNetworkDiagnostics::Structs::NeighborTableStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRThreadNetworkDiagnosticsClusterNeighborTableStruct * newElement_0;
            newElement_0 = [MTRThreadNetworkDiagnosticsClusterNeighborTableStruct new];
            newElement_0.extAddress = [NSNumber numberWithUnsignedLongLong:entry_0.extAddress];
            newElement_0.age = [NSNumber numberWithUnsignedInt:entry_0.age];
            newElement_0.rloc16 = [NSNumber numberWithUnsignedShort:entry_0.rloc16];
            newElement_0.linkFrameCounter = [NSNumber numberWithUnsignedInt:entry_0.linkFrameCounter];
            newElement_0.mleFrameCounter = [NSNumber numberWithUnsignedInt:entry_0.mleFrameCounter];
            newElement_0.lqi = [NSNumber numberWithUnsignedChar:entry_0.lqi];
            if (entry_0.averageRssi.IsNull()) {
                newElement_0.averageRssi = nil;
            } else {
                newElement_0.averageRssi = [NSNumber numberWithChar:entry_0.averageRssi.Value()];
            }
            if (entry_0.lastRssi.IsNull()) {
                newElement_0.lastRssi = nil;
            } else {
                newElement_0.lastRssi = [NSNumber numberWithChar:entry_0.lastRssi.Value()];
            }
            newElement_0.frameErrorRate = [NSNumber numberWithUnsignedChar:entry_0.frameErrorRate];
            newElement_0.messageErrorRate = [NSNumber numberWithUnsignedChar:entry_0.messageErrorRate];
            newElement_0.rxOnWhenIdle = [NSNumber numberWithBool:entry_0.rxOnWhenIdle];
            newElement_0.fullThreadDevice = [NSNumber numberWithBool:entry_0.fullThreadDevice];
            newElement_0.fullNetworkData = [NSNumber numberWithBool:entry_0.fullNetworkData];
            newElement_0.isChild = [NSNumber numberWithBool:entry_0.isChild];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsNeighborTableListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsRouteTableListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::ThreadNetworkDiagnostics::Structs::RouteTableStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRThreadNetworkDiagnosticsClusterRouteTableStruct * newElement_0;
            newElement_0 = [MTRThreadNetworkDiagnosticsClusterRouteTableStruct new];
            newElement_0.extAddress = [NSNumber numberWithUnsignedLongLong:entry_0.extAddress];
            newElement_0.rloc16 = [NSNumber numberWithUnsignedShort:entry_0.rloc16];
            newElement_0.routerId = [NSNumber numberWithUnsignedChar:entry_0.routerId];
            newElement_0.nextHop = [NSNumber numberWithUnsignedChar:entry_0.nextHop];
            newElement_0.pathCost = [NSNumber numberWithUnsignedChar:entry_0.pathCost];
            newElement_0.lqiIn = [NSNumber numberWithUnsignedChar:entry_0.LQIIn];
            newElement_0.lqiOut = [NSNumber numberWithUnsignedChar:entry_0.LQIOut];
            newElement_0.age = [NSNumber numberWithUnsignedChar:entry_0.age];
            newElement_0.allocated = [NSNumber numberWithBool:entry_0.allocated];
            newElement_0.linkEstablished = [NSNumber numberWithBool:entry_0.linkEstablished];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsRouteTableListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsSecurityPolicyStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::Structs::SecurityPolicy::DecodableType> &
        value)
{
    MTRThreadNetworkDiagnosticsClusterSecurityPolicy * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRThreadNetworkDiagnosticsClusterSecurityPolicy new];
        objCValue.rotationTime = [NSNumber numberWithUnsignedShort:value.Value().rotationTime];
        objCValue.flags = [NSNumber numberWithUnsignedShort:value.Value().flags];
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsSecurityPolicyStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsOperationalDatasetComponentsStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<
        chip::app::Clusters::ThreadNetworkDiagnostics::Structs::OperationalDatasetComponents::DecodableType> & value)
{
    MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents new];
        objCValue.activeTimestampPresent = [NSNumber numberWithBool:value.Value().activeTimestampPresent];
        objCValue.pendingTimestampPresent = [NSNumber numberWithBool:value.Value().pendingTimestampPresent];
        objCValue.masterKeyPresent = [NSNumber numberWithBool:value.Value().masterKeyPresent];
        objCValue.networkNamePresent = [NSNumber numberWithBool:value.Value().networkNamePresent];
        objCValue.extendedPanIdPresent = [NSNumber numberWithBool:value.Value().extendedPanIdPresent];
        objCValue.meshLocalPrefixPresent = [NSNumber numberWithBool:value.Value().meshLocalPrefixPresent];
        objCValue.delayPresent = [NSNumber numberWithBool:value.Value().delayPresent];
        objCValue.panIdPresent = [NSNumber numberWithBool:value.Value().panIdPresent];
        objCValue.channelPresent = [NSNumber numberWithBool:value.Value().channelPresent];
        objCValue.pskcPresent = [NSNumber numberWithBool:value.Value().pskcPresent];
        objCValue.securityPolicyPresent = [NSNumber numberWithBool:value.Value().securityPolicyPresent];
        objCValue.channelMaskPresent = [NSNumber numberWithBool:value.Value().channelMaskPresent];
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsOperationalDatasetComponentsStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFaultEnum> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeSynchronizationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTimeSynchronizationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationProductAppearanceStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::Clusters::BridgedDeviceBasicInformation::Structs::ProductAppearanceStruct::DecodableType & value)
{
    MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct * _Nonnull objCValue;
    objCValue = [MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct new];
    objCValue.finish = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.finish)];
    if (value.primaryColor.IsNull()) {
        objCValue.primaryColor = nil;
    } else {
        objCValue.primaryColor = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.primaryColor.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationProductAppearanceStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSwitchGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSwitchGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSwitchAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSwitchAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSwitchEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSwitchEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSwitchAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSwitchAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsNOCsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::OperationalCredentials::Structs::NOCStruct::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROperationalCredentialsClusterNOCStruct * newElement_0;
            newElement_0 = [MTROperationalCredentialsClusterNOCStruct new];
            newElement_0.noc = AsData(entry_0.noc);
            if (entry_0.icac.IsNull()) {
                newElement_0.icac = nil;
            } else {
                newElement_0.icac = AsData(entry_0.icac.Value());
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsNOCsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsFabricsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROperationalCredentialsClusterFabricDescriptorStruct * newElement_0;
            newElement_0 = [MTROperationalCredentialsClusterFabricDescriptorStruct new];
            newElement_0.rootPublicKey = AsData(entry_0.rootPublicKey);
            newElement_0.vendorID = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.vendorID)];
            newElement_0.fabricID = [NSNumber numberWithUnsignedLongLong:entry_0.fabricID];
            newElement_0.nodeID = [NSNumber numberWithUnsignedLongLong:entry_0.nodeID];
            newElement_0.label = AsString(entry_0.label);
            if (newElement_0.label == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsFabricsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsTrustedRootCertificatesListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::ByteSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSData * newElement_0;
            newElement_0 = AsData(entry_0);
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsTrustedRootCertificatesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGroupKeyMapListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::GroupKeyManagement::Structs::GroupKeyMapStruct::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRGroupKeyManagementClusterGroupKeyMapStruct * newElement_0;
            newElement_0 = [MTRGroupKeyManagementClusterGroupKeyMapStruct new];
            newElement_0.groupId = [NSNumber numberWithUnsignedShort:entry_0.groupId];
            newElement_0.groupKeySetID = [NSNumber numberWithUnsignedShort:entry_0.groupKeySetID];
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementGroupKeyMapListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGroupTableListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::GroupKeyManagement::Structs::GroupInfoMapStruct::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRGroupKeyManagementClusterGroupInfoMapStruct * newElement_0;
            newElement_0 = [MTRGroupKeyManagementClusterGroupInfoMapStruct new];
            newElement_0.groupId = [NSNumber numberWithUnsignedShort:entry_0.groupId];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.endpoints.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSNumber * newElement_2;
                    newElement_2 = [NSNumber numberWithUnsignedShort:entry_2];
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.endpoints = array_2;
            }
            if (entry_0.groupName.HasValue()) {
                newElement_0.groupName = AsString(entry_0.groupName.Value());
                if (newElement_0.groupName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.groupName = nil;
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementGroupTableListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFixedLabelLabelListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::FixedLabel::Structs::LabelStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRFixedLabelClusterLabelStruct * newElement_0;
            newElement_0 = [MTRFixedLabelClusterLabelStruct new];
            newElement_0.label = AsString(entry_0.label);
            if (newElement_0.label == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.value = AsString(entry_0.value);
            if (newElement_0.value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFixedLabelLabelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFixedLabelGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFixedLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFixedLabelAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFixedLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFixedLabelEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFixedLabelEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFixedLabelAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFixedLabelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUserLabelLabelListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::UserLabel::Structs::LabelStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRUserLabelClusterLabelStruct * newElement_0;
            newElement_0 = [MTRUserLabelClusterLabelStruct new];
            newElement_0.label = AsString(entry_0.label);
            if (newElement_0.label == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.value = AsString(entry_0.value);
            if (newElement_0.value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUserLabelLabelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUserLabelGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUserLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUserLabelAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUserLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUserLabelEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUserLabelEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUserLabelAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUserLabelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRProxyConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRProxyConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRProxyDiscoveryEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRProxyDiscoveryEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRProxyValidEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRProxyValidEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBooleanStateGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBooleanStateGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBooleanStateAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBooleanStateAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBooleanStateEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBooleanStateEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBooleanStateAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBooleanStateAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRModeSelectSupportedModesListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRModeSelectClusterModeOptionStruct * newElement_0;
            newElement_0 = [MTRModeSelectClusterModeOptionStruct new];
            newElement_0.label = AsString(entry_0.label);
            if (newElement_0.label == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.semanticTags.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    MTRModeSelectClusterSemanticTagStruct * newElement_2;
                    newElement_2 = [MTRModeSelectClusterSemanticTagStruct new];
                    newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode)];
                    newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.semanticTags = array_2;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRModeSelectSupportedModesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRModeSelectGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRModeSelectGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRModeSelectAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRModeSelectAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRModeSelectEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRModeSelectEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRModeSelectAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRModeSelectAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureControlSupportedTemperatureLevelsListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CharSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSString * newElement_0;
            newElement_0 = AsString(entry_0);
            if (newElement_0 == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureControlSupportedTemperatureLevelsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmMaskAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmMaskAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmSupportedAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmSupportedAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRefrigeratorAlarmAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRefrigeratorAlarmAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAirQualityGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAirQualityGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAirQualityAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAirQualityAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAirQualityEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAirQualityEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAirQualityAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAirQualityAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStatePhaseListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::DataModel::DecodableList<chip::CharSpan>> & value)
{
    NSArray * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = value.Value().begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSString * newElement_1;
                newElement_1 = AsString(entry_1);
                if (newElement_1 == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                OnFailureFn(context, err);
                return;
            }
            objCValue = array_1;
        }
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStatePhaseListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateOperationalStateListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::OperationalState::Structs::OperationalStateStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROperationalStateClusterOperationalStateStruct * newElement_0;
            newElement_0 = [MTROperationalStateClusterOperationalStateStruct new];
            newElement_0.operationalStateID = [NSNumber numberWithUnsignedChar:entry_0.operationalStateID];
            if (entry_0.operationalStateLabel.HasValue()) {
                newElement_0.operationalStateLabel = AsString(entry_0.operationalStateLabel.Value());
                if (newElement_0.operationalStateLabel == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.operationalStateLabel = nil;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateOperationalStateListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateOperationalStateStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalState::Structs::OperationalStateStruct::DecodableType & value)
{
    MTROperationalStateClusterOperationalStateStruct * _Nonnull objCValue;
    objCValue = [MTROperationalStateClusterOperationalStateStruct new];
    objCValue.operationalStateID = [NSNumber numberWithUnsignedChar:value.operationalStateID];
    if (value.operationalStateLabel.HasValue()) {
        objCValue.operationalStateLabel = AsString(value.operationalStateLabel.Value());
        if (objCValue.operationalStateLabel == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
    } else {
        objCValue.operationalStateLabel = nil;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateOperationalStateStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateOperationalErrorStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalState::Structs::ErrorStateStruct::DecodableType & value)
{
    MTROperationalStateClusterErrorStateStruct * _Nonnull objCValue;
    objCValue = [MTROperationalStateClusterErrorStateStruct new];
    objCValue.errorStateID = [NSNumber numberWithUnsignedChar:value.errorStateID];
    if (value.errorStateLabel.HasValue()) {
        objCValue.errorStateLabel = AsString(value.errorStateLabel.Value());
        if (objCValue.errorStateLabel == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
    } else {
        objCValue.errorStateLabel = nil;
    }
    if (value.errorStateDetails.HasValue()) {
        objCValue.errorStateDetails = AsString(value.errorStateDetails.Value());
        if (objCValue.errorStateDetails == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
    } else {
        objCValue.errorStateDetails = nil;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateOperationalErrorStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockCredentialRulesSupportAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockCredentialRulesSupportAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockSupportedOperatingModesAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockSupportedOperatingModesAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockDefaultConfigurationRegisterAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockDefaultConfigurationRegisterAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockLocalProgrammingFeaturesAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockLocalProgrammingFeaturesAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringConfigStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringConfigStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringOperationalStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringOperationalStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::Mode> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringSafetyStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringSafetyStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBarrierControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBarrierControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBarrierControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBarrierControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBarrierControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBarrierControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBarrierControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBarrierControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlPumpStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatusBitmap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlPumpStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFanControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFanControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFanControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFanControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatUserInterfaceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatUserInterfaceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatUserInterfaceConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRThermostatUserInterfaceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRColorControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRColorControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRColorControlEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRColorControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBallastConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBallastConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBallastConfigurationEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBallastConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTemperatureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPressureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPressureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPressureMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPressureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFlowMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFlowMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFlowMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFlowMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRelativeHumidityMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRelativeHumidityMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRelativeHumidityMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRelativeHumidityMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingOccupancyAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingOccupancyAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingOccupancySensorTypeBitmapAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancySensorTypeBitmap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingOccupancySensorTypeBitmapAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWakeOnLANGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLANGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWakeOnLANAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLANAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWakeOnLANEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLANEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWakeOnLANAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLANAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelChannelListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Channel::Structs::ChannelInfoStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRChannelClusterChannelInfoStruct * newElement_0;
            newElement_0 = [MTRChannelClusterChannelInfoStruct new];
            newElement_0.majorNumber = [NSNumber numberWithUnsignedShort:entry_0.majorNumber];
            newElement_0.minorNumber = [NSNumber numberWithUnsignedShort:entry_0.minorNumber];
            if (entry_0.name.HasValue()) {
                newElement_0.name = AsString(entry_0.name.Value());
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.name = nil;
            }
            if (entry_0.callSign.HasValue()) {
                newElement_0.callSign = AsString(entry_0.callSign.Value());
                if (newElement_0.callSign == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.callSign = nil;
            }
            if (entry_0.affiliateCallSign.HasValue()) {
                newElement_0.affiliateCallSign = AsString(entry_0.affiliateCallSign.Value());
                if (newElement_0.affiliateCallSign == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.affiliateCallSign = nil;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelChannelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelLineupStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::Structs::LineupInfoStruct::DecodableType> & value)
{
    MTRChannelClusterLineupInfoStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRChannelClusterLineupInfoStruct new];
        objCValue.operatorName = AsString(value.Value().operatorName);
        if (objCValue.operatorName == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
        if (value.Value().lineupName.HasValue()) {
            objCValue.lineupName = AsString(value.Value().lineupName.Value());
            if (objCValue.lineupName == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
        } else {
            objCValue.lineupName = nil;
        }
        if (value.Value().postalCode.HasValue()) {
            objCValue.postalCode = AsString(value.Value().postalCode.Value());
            if (objCValue.postalCode == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
        } else {
            objCValue.postalCode = nil;
        }
        objCValue.lineupInfoType = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value().lineupInfoType)];
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelLineupStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelCurrentChannelStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::Structs::ChannelInfoStruct::DecodableType> & value)
{
    MTRChannelClusterChannelInfoStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRChannelClusterChannelInfoStruct new];
        objCValue.majorNumber = [NSNumber numberWithUnsignedShort:value.Value().majorNumber];
        objCValue.minorNumber = [NSNumber numberWithUnsignedShort:value.Value().minorNumber];
        if (value.Value().name.HasValue()) {
            objCValue.name = AsString(value.Value().name.Value());
            if (objCValue.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
        } else {
            objCValue.name = nil;
        }
        if (value.Value().callSign.HasValue()) {
            objCValue.callSign = AsString(value.Value().callSign.Value());
            if (objCValue.callSign == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
        } else {
            objCValue.callSign = nil;
        }
        if (value.Value().affiliateCallSign.HasValue()) {
            objCValue.affiliateCallSign = AsString(value.Value().affiliateCallSign.Value());
            if (objCValue.affiliateCallSign == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
        } else {
            objCValue.affiliateCallSign = nil;
        }
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelCurrentChannelStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRChannelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorTargetListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRTargetNavigatorClusterTargetInfoStruct * newElement_0;
            newElement_0 = [MTRTargetNavigatorClusterTargetInfoStruct new];
            newElement_0.identifier = [NSNumber numberWithUnsignedChar:entry_0.identifier];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorTargetListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackSampledPositionStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::MediaPlayback::Structs::PlaybackPositionStruct::DecodableType> &
        value)
{
    MTRMediaPlaybackClusterPlaybackPositionStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRMediaPlaybackClusterPlaybackPositionStruct new];
        objCValue.updatedAt = [NSNumber numberWithUnsignedLongLong:value.Value().updatedAt];
        if (value.Value().position.IsNull()) {
            objCValue.position = nil;
        } else {
            objCValue.position = [NSNumber numberWithUnsignedLongLong:value.Value().position.Value()];
        }
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackSampledPositionStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputInputListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::MediaInput::Structs::InputInfoStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRMediaInputClusterInputInfoStruct * newElement_0;
            newElement_0 = [MTRMediaInputClusterInputInfoStruct new];
            newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
            newElement_0.inputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.inputType)];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.descriptionString = AsString(entry_0.description);
            if (newElement_0.descriptionString == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputInputListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLowPowerGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLowPowerGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLowPowerAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLowPowerAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLowPowerEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLowPowerEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLowPowerAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRLowPowerAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAcceptHeaderListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CharSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSString * newElement_0;
            newElement_0 = AsString(entry_0);
            if (newElement_0 == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherAcceptHeaderListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputOutputListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::AudioOutput::Structs::OutputInfoStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAudioOutputClusterOutputInfoStruct * newElement_0;
            newElement_0 = [MTRAudioOutputClusterOutputInfoStruct new];
            newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
            newElement_0.outputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.outputType)];
            newElement_0.name = AsString(entry_0.name);
            if (newElement_0.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputOutputListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherCatalogListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<uint16_t> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherCatalogListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherCurrentAppStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEPStruct::DecodableType> &
        value)
{
    MTRApplicationLauncherClusterApplicationEPStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRApplicationLauncherClusterApplicationEPStruct new];
        objCValue.application = [MTRApplicationLauncherClusterApplicationStruct new];
        objCValue.application.catalogVendorID = [NSNumber numberWithUnsignedShort:value.Value().application.catalogVendorID];
        objCValue.application.applicationID = AsString(value.Value().application.applicationID);
        if (objCValue.application.applicationID == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
        if (value.Value().endpoint.HasValue()) {
            objCValue.endpoint = [NSNumber numberWithUnsignedShort:value.Value().endpoint.Value()];
        } else {
            objCValue.endpoint = nil;
        }
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherCurrentAppStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicApplicationStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::ApplicationBasic::Structs::ApplicationStruct::DecodableType & value)
{
    MTRApplicationBasicClusterApplicationStruct * _Nonnull objCValue;
    objCValue = [MTRApplicationBasicClusterApplicationStruct new];
    objCValue.catalogVendorID = [NSNumber numberWithUnsignedShort:value.catalogVendorID];
    objCValue.applicationID = AsString(value.applicationID);
    if (objCValue.applicationID == nil) {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicApplicationStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAllowedVendorListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::VendorId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicAllowedVendorListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccountLoginGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccountLoginGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccountLoginAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccountLoginAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccountLoginEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccountLoginEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccountLoginAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRAccountLoginAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRElectricalMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRElectricalMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRElectricalMeasurementEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRElectricalMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingBitmap8AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap8MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingBitmap8AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingBitmap16AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap16MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingBitmap16AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingBitmap32AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap32MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingBitmap32AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingBitmap64AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap64MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedLongLong:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingBitmap64AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListInt8uListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<uint8_t> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListInt8uListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListOctetStringListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::ByteSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSData * newElement_0;
            newElement_0 = AsData(entry_0);
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListStructOctetStringListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::UnitTesting::Structs::TestListStructOctet::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRUnitTestingClusterTestListStructOctet * newElement_0;
            newElement_0 = [MTRUnitTestingClusterTestListStructOctet new];
            newElement_0.member1 = [NSNumber numberWithUnsignedLongLong:entry_0.member1];
            newElement_0.member2 = AsData(entry_0.member2);
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListStructOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListNullablesAndOptionalsStructListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRUnitTestingClusterNullablesAndOptionalsStruct * newElement_0;
            newElement_0 = [MTRUnitTestingClusterNullablesAndOptionalsStruct new];
            if (entry_0.nullableInt.IsNull()) {
                newElement_0.nullableInt = nil;
            } else {
                newElement_0.nullableInt = [NSNumber numberWithUnsignedShort:entry_0.nullableInt.Value()];
            }
            if (entry_0.optionalInt.HasValue()) {
                newElement_0.optionalInt = [NSNumber numberWithUnsignedShort:entry_0.optionalInt.Value()];
            } else {
                newElement_0.optionalInt = nil;
            }
            if (entry_0.nullableOptionalInt.HasValue()) {
                if (entry_0.nullableOptionalInt.Value().IsNull()) {
                    newElement_0.nullableOptionalInt = nil;
                } else {
                    newElement_0.nullableOptionalInt =
                        [NSNumber numberWithUnsignedShort:entry_0.nullableOptionalInt.Value().Value()];
                }
            } else {
                newElement_0.nullableOptionalInt = nil;
            }
            if (entry_0.nullableString.IsNull()) {
                newElement_0.nullableString = nil;
            } else {
                newElement_0.nullableString = AsString(entry_0.nullableString.Value());
                if (newElement_0.nullableString == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            }
            if (entry_0.optionalString.HasValue()) {
                newElement_0.optionalString = AsString(entry_0.optionalString.Value());
                if (newElement_0.optionalString == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
            } else {
                newElement_0.optionalString = nil;
            }
            if (entry_0.nullableOptionalString.HasValue()) {
                if (entry_0.nullableOptionalString.Value().IsNull()) {
                    newElement_0.nullableOptionalString = nil;
                } else {
                    newElement_0.nullableOptionalString = AsString(entry_0.nullableOptionalString.Value().Value());
                    if (newElement_0.nullableOptionalString == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        OnFailureFn(context, err);
                        return;
                    }
                }
            } else {
                newElement_0.nullableOptionalString = nil;
            }
            if (entry_0.nullableStruct.IsNull()) {
                newElement_0.nullableStruct = nil;
            } else {
                newElement_0.nullableStruct = [MTRUnitTestingClusterSimpleStruct new];
                newElement_0.nullableStruct.a = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().a];
                newElement_0.nullableStruct.b = [NSNumber numberWithBool:entry_0.nullableStruct.Value().b];
                newElement_0.nullableStruct.c =
                    [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableStruct.Value().c)];
                newElement_0.nullableStruct.d = AsData(entry_0.nullableStruct.Value().d);
                newElement_0.nullableStruct.e = AsString(entry_0.nullableStruct.Value().e);
                if (newElement_0.nullableStruct.e == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.nullableStruct.f = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().f.Raw()];
                newElement_0.nullableStruct.g = [NSNumber numberWithFloat:entry_0.nullableStruct.Value().g];
                newElement_0.nullableStruct.h = [NSNumber numberWithDouble:entry_0.nullableStruct.Value().h];
            }
            if (entry_0.optionalStruct.HasValue()) {
                newElement_0.optionalStruct = [MTRUnitTestingClusterSimpleStruct new];
                newElement_0.optionalStruct.a = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().a];
                newElement_0.optionalStruct.b = [NSNumber numberWithBool:entry_0.optionalStruct.Value().b];
                newElement_0.optionalStruct.c =
                    [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.optionalStruct.Value().c)];
                newElement_0.optionalStruct.d = AsData(entry_0.optionalStruct.Value().d);
                newElement_0.optionalStruct.e = AsString(entry_0.optionalStruct.Value().e);
                if (newElement_0.optionalStruct.e == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.optionalStruct.f = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().f.Raw()];
                newElement_0.optionalStruct.g = [NSNumber numberWithFloat:entry_0.optionalStruct.Value().g];
                newElement_0.optionalStruct.h = [NSNumber numberWithDouble:entry_0.optionalStruct.Value().h];
            } else {
                newElement_0.optionalStruct = nil;
            }
            if (entry_0.nullableOptionalStruct.HasValue()) {
                if (entry_0.nullableOptionalStruct.Value().IsNull()) {
                    newElement_0.nullableOptionalStruct = nil;
                } else {
                    newElement_0.nullableOptionalStruct = [MTRUnitTestingClusterSimpleStruct new];
                    newElement_0.nullableOptionalStruct.a =
                        [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().a];
                    newElement_0.nullableOptionalStruct.b =
                        [NSNumber numberWithBool:entry_0.nullableOptionalStruct.Value().Value().b];
                    newElement_0.nullableOptionalStruct.c =
                        [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableOptionalStruct.Value().Value().c)];
                    newElement_0.nullableOptionalStruct.d = AsData(entry_0.nullableOptionalStruct.Value().Value().d);
                    newElement_0.nullableOptionalStruct.e = AsString(entry_0.nullableOptionalStruct.Value().Value().e);
                    if (newElement_0.nullableOptionalStruct.e == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        OnFailureFn(context, err);
                        return;
                    }
                    newElement_0.nullableOptionalStruct.f =
                        [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().f.Raw()];
                    newElement_0.nullableOptionalStruct.g =
                        [NSNumber numberWithFloat:entry_0.nullableOptionalStruct.Value().Value().g];
                    newElement_0.nullableOptionalStruct.h =
                        [NSNumber numberWithDouble:entry_0.nullableOptionalStruct.Value().Value().h];
                }
            } else {
                newElement_0.nullableOptionalStruct = nil;
            }
            if (entry_0.nullableList.IsNull()) {
                newElement_0.nullableList = nil;
            } else {
                { // Scope for our temporary variables
                    auto * array_3 = [NSMutableArray new];
                    auto iter_3 = entry_0.nullableList.Value().begin();
                    while (iter_3.Next()) {
                        auto & entry_3 = iter_3.GetValue();
                        NSNumber * newElement_3;
                        newElement_3 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_3)];
                        [array_3 addObject:newElement_3];
                    }
                    CHIP_ERROR err = iter_3.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        OnFailureFn(context, err);
                        return;
                    }
                    newElement_0.nullableList = array_3;
                }
            }
            if (entry_0.optionalList.HasValue()) {
                { // Scope for our temporary variables
                    auto * array_3 = [NSMutableArray new];
                    auto iter_3 = entry_0.optionalList.Value().begin();
                    while (iter_3.Next()) {
                        auto & entry_3 = iter_3.GetValue();
                        NSNumber * newElement_3;
                        newElement_3 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_3)];
                        [array_3 addObject:newElement_3];
                    }
                    CHIP_ERROR err = iter_3.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        OnFailureFn(context, err);
                        return;
                    }
                    newElement_0.optionalList = array_3;
                }
            } else {
                newElement_0.optionalList = nil;
            }
            if (entry_0.nullableOptionalList.HasValue()) {
                if (entry_0.nullableOptionalList.Value().IsNull()) {
                    newElement_0.nullableOptionalList = nil;
                } else {
                    { // Scope for our temporary variables
                        auto * array_4 = [NSMutableArray new];
                        auto iter_4 = entry_0.nullableOptionalList.Value().Value().begin();
                        while (iter_4.Next()) {
                            auto & entry_4 = iter_4.GetValue();
                            NSNumber * newElement_4;
                            newElement_4 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_4)];
                            [array_4 addObject:newElement_4];
                        }
                        CHIP_ERROR err = iter_4.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            OnFailureFn(context, err);
                            return;
                        }
                        newElement_0.nullableOptionalList = array_4;
                    }
                }
            } else {
                newElement_0.nullableOptionalList = nil;
            }
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListNullablesAndOptionalsStructListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingStructAttrStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Structs::SimpleStruct::DecodableType & value)
{
    MTRUnitTestingClusterSimpleStruct * _Nonnull objCValue;
    objCValue = [MTRUnitTestingClusterSimpleStruct new];
    objCValue.a = [NSNumber numberWithUnsignedChar:value.a];
    objCValue.b = [NSNumber numberWithBool:value.b];
    objCValue.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.c)];
    objCValue.d = AsData(value.d);
    objCValue.e = AsString(value.e);
    if (objCValue.e == nil) {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
        OnFailureFn(context, err);
        return;
    }
    objCValue.f = [NSNumber numberWithUnsignedChar:value.f.Raw()];
    objCValue.g = [NSNumber numberWithFloat:value.g];
    objCValue.h = [NSNumber numberWithDouble:value.h];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingStructAttrStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListLongOctetStringListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::ByteSpan> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSData * newElement_0;
            newElement_0 = AsData(entry_0);
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListLongOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingListFabricScopedListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::UnitTesting::Structs::TestFabricScoped::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRUnitTestingClusterTestFabricScoped * newElement_0;
            newElement_0 = [MTRUnitTestingClusterTestFabricScoped new];
            newElement_0.fabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveInt8u];
            if (entry_0.optionalFabricSensitiveInt8u.HasValue()) {
                newElement_0.optionalFabricSensitiveInt8u =
                    [NSNumber numberWithUnsignedChar:entry_0.optionalFabricSensitiveInt8u.Value()];
            } else {
                newElement_0.optionalFabricSensitiveInt8u = nil;
            }
            if (entry_0.nullableFabricSensitiveInt8u.IsNull()) {
                newElement_0.nullableFabricSensitiveInt8u = nil;
            } else {
                newElement_0.nullableFabricSensitiveInt8u =
                    [NSNumber numberWithUnsignedChar:entry_0.nullableFabricSensitiveInt8u.Value()];
            }
            if (entry_0.nullableOptionalFabricSensitiveInt8u.HasValue()) {
                if (entry_0.nullableOptionalFabricSensitiveInt8u.Value().IsNull()) {
                    newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
                } else {
                    newElement_0.nullableOptionalFabricSensitiveInt8u =
                        [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalFabricSensitiveInt8u.Value().Value()];
                }
            } else {
                newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
            }
            newElement_0.fabricSensitiveCharString = AsString(entry_0.fabricSensitiveCharString);
            if (newElement_0.fabricSensitiveCharString == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.fabricSensitiveStruct = [MTRUnitTestingClusterSimpleStruct new];
            newElement_0.fabricSensitiveStruct.a = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.a];
            newElement_0.fabricSensitiveStruct.b = [NSNumber numberWithBool:entry_0.fabricSensitiveStruct.b];
            newElement_0.fabricSensitiveStruct.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.fabricSensitiveStruct.c)];
            newElement_0.fabricSensitiveStruct.d = AsData(entry_0.fabricSensitiveStruct.d);
            newElement_0.fabricSensitiveStruct.e = AsString(entry_0.fabricSensitiveStruct.e);
            if (newElement_0.fabricSensitiveStruct.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                OnFailureFn(context, err);
                return;
            }
            newElement_0.fabricSensitiveStruct.f = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.f.Raw()];
            newElement_0.fabricSensitiveStruct.g = [NSNumber numberWithFloat:entry_0.fabricSensitiveStruct.g];
            newElement_0.fabricSensitiveStruct.h = [NSNumber numberWithDouble:entry_0.fabricSensitiveStruct.h];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.fabricSensitiveInt8uList.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSNumber * newElement_2;
                    newElement_2 = [NSNumber numberWithUnsignedChar:entry_2];
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                newElement_0.fabricSensitiveInt8uList = array_2;
            }
            newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingListFabricScopedListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingNullableBitmap8AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap8MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingNullableBitmap8AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingNullableBitmap16AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap16MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingNullableBitmap16AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingNullableBitmap32AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap32MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedInt:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingNullableBitmap32AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingNullableBitmap64AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::UnitTesting::Bitmap64MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedLongLong:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingNullableBitmap64AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingNullableStructStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::UnitTesting::Structs::SimpleStruct::DecodableType> & value)
{
    MTRUnitTestingClusterSimpleStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRUnitTestingClusterSimpleStruct new];
        objCValue.a = [NSNumber numberWithUnsignedChar:value.Value().a];
        objCValue.b = [NSNumber numberWithBool:value.Value().b];
        objCValue.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value().c)];
        objCValue.d = AsData(value.Value().d);
        objCValue.e = AsString(value.Value().e);
        if (objCValue.e == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            OnFailureFn(context, err);
            return;
        }
        objCValue.f = [NSNumber numberWithUnsignedChar:value.Value().f.Raw()];
        objCValue.g = [NSNumber numberWithFloat:value.Value().g];
        objCValue.h = [NSNumber numberWithDouble:value.Value().h];
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingNullableStructStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingAttributeListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::AttributeId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFaultInjectionEventListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::EventId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRFaultInjectionEventListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupsClusterAddGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterAddGroupResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterViewGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterViewGroupResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterGetGroupMembershipResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterGetGroupMembershipResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterRemoveGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterRemoveGroupResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterAddSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterAddSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterViewSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterViewSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterRemoveSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterRemoveSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterRemoveAllScenesResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterRemoveAllScenesResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterStoreSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterStoreSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterGetSceneMembershipResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterGetSceneMembershipResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterEnhancedAddSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterEnhancedAddSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterEnhancedViewSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterEnhancedViewSceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterCopySceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterCopySceneResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROTASoftwareUpdateProviderClusterQueryImageResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType & data)
{
    auto * response = [MTROTASoftwareUpdateProviderClusterQueryImageResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROTASoftwareUpdateProviderClusterApplyUpdateResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType & data)
{
    auto * response = [MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterArmFailSafeResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterArmFailSafeResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterSetRegulatoryConfigResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterCommissioningCompleteResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterCommissioningCompleteResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterScanNetworksResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterScanNetworksResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterNetworkConfigResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterNetworkConfigResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterConnectNetworkResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterConnectNetworkResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDiagnosticLogsClusterRetrieveLogsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType & data)
{
    auto * response = [MTRDiagnosticLogsClusterRetrieveLogsResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterAttestationResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterAttestationResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterCertificateChainResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterCertificateChainResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterCSRResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterCSRResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterNOCResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterNOCResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGroupKeyManagementClusterKeySetReadResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType & data)
{
    auto * response = [MTRGroupKeyManagementClusterKeySetReadResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType & data)
{
    auto * response = [MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTROperationalStateClusterOperationalCommandResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalState::Commands::OperationalCommandResponse::DecodableType & data)
{
    auto * response = [MTROperationalStateClusterOperationalCommandResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetWeekDayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetWeekDayScheduleResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetYearDayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetYearDayScheduleResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetHolidayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetHolidayScheduleResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetUserResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetUserResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterSetCredentialResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterSetCredentialResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetCredentialStatusResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetCredentialStatusResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRThermostatClusterGetWeeklyScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType & data)
{
    auto * response = [MTRThermostatClusterGetWeeklyScheduleResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRChannelClusterChangeChannelResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType & data)
{
    auto * response = [MTRChannelClusterChangeChannelResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRTargetNavigatorClusterNavigateTargetResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType & data)
{
    auto * response = [MTRTargetNavigatorClusterNavigateTargetResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRMediaPlaybackClusterPlaybackResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType & data)
{
    auto * response = [MTRMediaPlaybackClusterPlaybackResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRKeypadInputClusterSendKeyResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType & data)
{
    auto * response = [MTRKeypadInputClusterSendKeyResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRContentLauncherClusterLauncherResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType & data)
{
    auto * response = [MTRContentLauncherClusterLauncherResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRApplicationLauncherClusterLauncherResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType & data)
{
    auto * response = [MTRApplicationLauncherClusterLauncherResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRAccountLoginClusterGetSetupPINResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType & data)
{
    auto * response = [MTRAccountLoginClusterGetSetupPINResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestSpecificResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestSpecificResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestAddArgumentsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestAddArgumentsResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestSimpleArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestSimpleArgumentResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestStructArrayArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestStructArrayArgumentResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestListInt8UReverseResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestListInt8UReverseResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEnumsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEnumsResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestNullableOptionalResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestComplexNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestComplexNullableOptionalResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterBooleanResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterBooleanResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterSimpleStructResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterSimpleStructResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEmitTestEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEmitTestEventResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams new];
    CHIP_ERROR err = [response _setFieldsFromDecodableStruct:data];
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    DispatchSuccess(context, response);
};

void MTRIdentifyClusterEffectIdentifierEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::EffectIdentifierEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterEffectIdentifierEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterEffectIdentifierEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::EffectIdentifierEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterEffectIdentifierEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterEffectVariantEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::EffectVariantEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterEffectVariantEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterEffectVariantEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::EffectVariantEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterEffectVariantEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::IdentifyTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterIdentifyTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffDyingLightEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffEffectIdentifierAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffEffectIdentifier value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffEffectIdentifierAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffEffectIdentifier> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffStartUpOnOffAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffStartUpOnOff value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffStartUpOnOffAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlClusterMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::LevelControl::MoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableLevelControlClusterMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::LevelControl::MoveMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRLevelControlClusterStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::LevelControl::StepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableLevelControlClusterStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::LevelControl::StepMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterAccessControlEntryAuthModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::AccessControlEntryAuthModeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterAccessControlEntryAuthModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterAccessControlEntryAuthModeEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::AccessControl::AccessControlEntryAuthModeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAccessControlClusterAccessControlEntryAuthModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterAccessControlEntryPrivilegeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::AccessControlEntryPrivilegeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterAccessControlEntryPrivilegeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterAccessControlEntryPrivilegeEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::AccessControl::AccessControlEntryPrivilegeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAccessControlClusterAccessControlEntryPrivilegeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterChangeTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::ChangeTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterChangeTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AccessControl::ChangeTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionErrorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Actions::ActionErrorEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Actions::ActionStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Actions::ActionTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActionsClusterEndpointListTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::EndpointListTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterEndpointListTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Actions::EndpointListTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationClusterColorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::BasicInformation::ColorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationClusterColorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableBasicInformationClusterColorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::BasicInformation::ColorEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableBasicInformationClusterColorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBasicInformationClusterProductFinishEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::BasicInformation::ProductFinishEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRBasicInformationClusterProductFinishEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableBasicInformationClusterProductFinishEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::BasicInformation::ProductFinishEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableBasicInformationClusterProductFinishEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROTASoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROTASoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOTASoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOTASoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationClusterCalendarTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterCalendarTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterCalendarTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeFormatLocalizationClusterCalendarTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationClusterHourFormatEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::HourFormatEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterHourFormatEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterHourFormatEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeFormatLocalization::HourFormatEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeFormatLocalizationClusterHourFormatEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationClusterTempUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::UnitLocalization::TempUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationClusterTempUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableUnitLocalizationClusterTempUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::UnitLocalization::TempUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableUnitLocalizationClusterTempUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatApprovedChemistryEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatApprovedChemistryEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatApprovedChemistryEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatApprovedChemistryEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatApprovedChemistryEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatApprovedChemistryEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeLevelEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeLevelEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeLevelEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeLevelEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeLevelEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeLevelEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatCommonDesignationEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatCommonDesignationEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatCommonDesignationEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatCommonDesignationEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatCommonDesignationEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatCommonDesignationEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatReplaceabilityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatReplaceabilityEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatReplaceabilityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatReplaceabilityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatReplaceabilityEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatReplaceabilityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterPowerSourceStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::PowerSourceStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterPowerSourceStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterPowerSourceStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::PowerSourceStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterPowerSourceStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredCurrentTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredCurrentTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredCurrentTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredCurrentTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::WiredCurrentTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterWiredCurrentTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::WiredFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterWiredFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterCommissioningErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterCommissioningErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterCommissioningErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralCommissioningClusterCommissioningErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterRegulatoryLocationTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterRegulatoryLocationTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningClusterNetworkCommissioningStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterNetworkCommissioningStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningClusterWiFiBandEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::WiFiBandEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterWiFiBandEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterWiFiBandEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::WiFiBandEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNetworkCommissioningClusterWiFiBandEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterIntentEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::IntentEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterIntentEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterIntentEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::IntentEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterIntentEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::StatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::StatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterTransferProtocolEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::TransferProtocolEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterTransferProtocolEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterTransferProtocolEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::TransferProtocolEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterTransferProtocolEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterBootReasonEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::BootReasonEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterBootReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterBootReasonEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::BootReasonEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterBootReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterHardwareFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::HardwareFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterHardwareFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterHardwareFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::HardwareFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterHardwareFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterInterfaceTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::InterfaceTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterInterfaceTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterInterfaceTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::InterfaceTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterInterfaceTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterNetworkFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::NetworkFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterNetworkFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterNetworkFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::NetworkFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterNetworkFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterRadioFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::RadioFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterRadioFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterRadioFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::RadioFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterRadioFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::ConnectionStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::ConnectionStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterNetworkFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFaultEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterNetworkFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFaultEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterRoutingRoleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRoleEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterRoutingRoleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRoleEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterConnectionStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsClusterPHYRateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsClusterPHYRateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableEthernetNetworkDiagnosticsClusterPHYRateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableEthernetNetworkDiagnosticsClusterPHYRateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeSynchronization::GranularityEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeSynchronizationClusterGranularityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeSynchronization::GranularityEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeSynchronization::TimeSourceEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeSynchronizationClusterTimeSourceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeSynchronization::TimeSourceEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationClusterColorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::BridgedDeviceBasicInformation::ColorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationClusterColorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableBridgedDeviceBasicInformationClusterColorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::BridgedDeviceBasicInformation::ColorEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableBridgedDeviceBasicInformationClusterColorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicInformationClusterProductFinishEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::BridgedDeviceBasicInformation::ProductFinishEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicInformationClusterProductFinishEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableBridgedDeviceBasicInformationClusterProductFinishEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::BridgedDeviceBasicInformation::ProductFinishEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableBridgedDeviceBasicInformationClusterProductFinishEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningClusterCommissioningWindowStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningClusterCommissioningWindowStatusEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AdministratorCommissioning::StatusCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAdministratorCommissioningClusterStatusCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AdministratorCommissioning::StatusCode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsClusterCertificateChainTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OperationalCredentials::CertificateChainTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsClusterCertificateChainTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOperationalCredentialsClusterCertificateChainTypeEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OperationalCredentials::CertificateChainTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOperationalCredentialsClusterCertificateChainTypeEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsClusterNodeOperationalCertStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OperationalCredentials::NodeOperationalCertStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsClusterNodeOperationalCertStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOperationalCredentialsClusterNodeOperationalCertStatusEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OperationalCredentials::NodeOperationalCertStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOperationalCredentialsClusterNodeOperationalCertStatusEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicyEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicyEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAirQualityClusterAirQualityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AirQuality::AirQualityEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAirQualityClusterAirQualityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAirQualityClusterAirQualityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AirQuality::AirQualityEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAirQualityClusterAirQualityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterAlarmStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterAlarmStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterAlarmStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterAlarmStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterContaminationStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::ContaminationStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterContaminationStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterContaminationStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::ContaminationStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterContaminationStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterEndOfServiceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::EndOfServiceEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterEndOfServiceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterEndOfServiceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::EndOfServiceEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterEndOfServiceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterExpressedStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterExpressedStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterExpressedStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterExpressedStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterMuteStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::MuteStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterMuteStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterMuteStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::MuteStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterMuteStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRSmokeCOAlarmClusterSensitivityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::SmokeCoAlarm::SensitivityEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRSmokeCOAlarmClusterSensitivityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableSmokeCOAlarmClusterSensitivityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::SmokeCoAlarm::SensitivityEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableSmokeCOAlarmClusterSensitivityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateClusterErrorStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OperationalState::ErrorStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateClusterErrorStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOperationalStateClusterErrorStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OperationalState::ErrorStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOperationalStateClusterErrorStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROperationalStateClusterOperationalStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OperationalState::OperationalStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROperationalStateClusterOperationalStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOperationalStateClusterOperationalStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OperationalState::OperationalStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOperationalStateClusterOperationalStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringClusterChangeIndicationEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::HepaFilterMonitoring::ChangeIndicationEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringClusterChangeIndicationEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableHEPAFilterMonitoringClusterChangeIndicationEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::HepaFilterMonitoring::ChangeIndicationEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableHEPAFilterMonitoringClusterChangeIndicationEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRHEPAFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::HepaFilterMonitoring::DegradationDirectionEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRHEPAFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableHEPAFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::HepaFilterMonitoring::DegradationDirectionEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableHEPAFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringClusterChangeIndicationEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ActivatedCarbonFilterMonitoring::ChangeIndicationEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringClusterChangeIndicationEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActivatedCarbonFilterMonitoringClusterChangeIndicationEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ActivatedCarbonFilterMonitoring::ChangeIndicationEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActivatedCarbonFilterMonitoringClusterChangeIndicationEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRActivatedCarbonFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ActivatedCarbonFilterMonitoring::DegradationDirectionEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActivatedCarbonFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableActivatedCarbonFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ActivatedCarbonFilterMonitoring::DegradationDirectionEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableActivatedCarbonFilterMonitoringClusterDegradationDirectionEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterAlarmCodeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::AlarmCodeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterAlarmCodeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterAlarmCodeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::AlarmCodeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterAlarmCodeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterCredentialRuleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::CredentialRuleEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterCredentialRuleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterCredentialRuleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::CredentialRuleEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterCredentialRuleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterCredentialTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::CredentialTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterCredentialTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterCredentialTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::CredentialTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterCredentialTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDataOperationTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DataOperationTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDataOperationTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDataOperationTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DataOperationTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDataOperationTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockState value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockStateAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlStatusAttributeCallbackBridge::OnSuccessFn(void * context, chip::app::Clusters::DoorLock::DlStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockOperationEventCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockOperationEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorLockOperationEventCode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterLockDataTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::LockDataTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterLockDataTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterLockDataTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::LockDataTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterLockDataTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterLockOperationTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::LockOperationTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterLockOperationTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterLockOperationTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::LockOperationTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterLockOperationTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterOperatingModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::OperatingModeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterOperatingModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterOperatingModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::OperatingModeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterOperatingModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterOperationErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::OperationErrorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterOperationErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterOperationErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::OperationErrorEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterOperationErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterOperationSourceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::OperationSourceEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterOperationSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterOperationSourceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::OperationSourceEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterOperationSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterUserStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::UserStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterUserStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterUserStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::UserStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterUserStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterUserTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::UserTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterUserTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterUserTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::UserTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterUserTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringClusterEndProductTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WindowCovering::EndProductType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWindowCoveringClusterEndProductTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WindowCovering::EndProductType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringClusterTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WindowCovering::Type value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWindowCoveringClusterTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WindowCovering::Type> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlClusterControlModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::ControlModeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterControlModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterControlModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PumpConfigurationAndControl::ControlModeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePumpConfigurationAndControlClusterControlModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlClusterOperationModeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::OperationModeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterOperationModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterOperationModeEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::PumpConfigurationAndControl::OperationModeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePumpConfigurationAndControlClusterOperationModeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterSetpointAdjustModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::SetpointAdjustMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterSetpointAdjustModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Thermostat::SetpointAdjustMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatControlSequenceAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatControlSequence value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatControlSequenceAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Thermostat::ThermostatControlSequence> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatRunningModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatRunningMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatRunningModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Thermostat::ThermostatRunningMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatSystemModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatSystemMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatSystemModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Thermostat::ThermostatSystemMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterAirflowDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::AirflowDirectionEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterAirflowDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterAirflowDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::FanControl::AirflowDirectionEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFanControlClusterAirflowDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::DirectionEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterDirectionEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::FanControl::DirectionEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFanControlClusterDirectionEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterFanModeSequenceTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::FanModeSequenceType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterFanModeSequenceTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::FanControl::FanModeSequenceType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterFanModeTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::FanModeType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterFanModeTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::FanControl::FanModeType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorLoopActionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorLoopAction value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorLoopActionAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::ColorLoopAction> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorLoopDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorLoopDirection value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorLoopDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::ColorLoopDirection> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::ColorMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueDirection value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::HueDirection> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueMoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::HueMoveMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueStepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::HueStepMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterSaturationMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::SaturationMoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterSaturationMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::SaturationMoveMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterSaturationStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::SaturationStepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterSaturationStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ColorControl::SaturationStepMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementClusterLightSensorTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::IlluminanceMeasurement::LightSensorTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementClusterLightSensorTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::IlluminanceMeasurement::LightSensorTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROccupancySensingClusterOccupancySensorTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OccupancySensing::OccupancySensorTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROccupancySensingClusterOccupancySensorTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOccupancySensingClusterOccupancySensorTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::OccupancySensing::OccupancySensorTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOccupancySensingClusterOccupancySensorTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::MeasurementMediumEnum> &
        value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonMonoxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonMonoxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonMonoxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonDioxideConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonDioxideConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonDioxideConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonDioxideConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRCarbonDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::CarbonDioxideConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRCarbonDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableCarbonDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::CarbonDioxideConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableCarbonDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::MeasurementMediumEnum> &
        value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNitrogenDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNitrogenDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNitrogenDioxideConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OzoneConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOzoneConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OzoneConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOzoneConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OzoneConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOzoneConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OzoneConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOzoneConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTROzoneConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OzoneConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROzoneConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableOzoneConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OzoneConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOzoneConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm25ConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM25ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Pm25ConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM25ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm25ConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM25ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm25ConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM25ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM25ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm25ConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM25ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM25ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm25ConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM25ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FormaldehydeConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFormaldehydeConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::FormaldehydeConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFormaldehydeConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FormaldehydeConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFormaldehydeConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::FormaldehydeConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFormaldehydeConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFormaldehydeConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FormaldehydeConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFormaldehydeConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFormaldehydeConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::FormaldehydeConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFormaldehydeConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm1ConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM1ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Pm1ConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM1ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm1ConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM1ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm1ConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM1ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM1ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm1ConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM1ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM1ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm1ConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM1ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm10ConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM10ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Pm10ConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM10ConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm10ConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM10ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm10ConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM10ConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPM10ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Pm10ConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPM10ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePM10ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::Pm10ConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePM10ConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<
        chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::
    OnSuccessFn(void * context,
        const chip::app::DataModel::Nullable<
            chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context,
    const chip::app::DataModel::Nullable<
        chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTotalVolatileOrganicCompoundsConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::RadonConcentrationMeasurement::LevelValueEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableRadonConcentrationMeasurementClusterLevelValueEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::RadonConcentrationMeasurement::LevelValueEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableRadonConcentrationMeasurementClusterLevelValueEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::RadonConcentrationMeasurement::MeasurementMediumEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableRadonConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::RadonConcentrationMeasurement::MeasurementMediumEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableRadonConcentrationMeasurementClusterMeasurementMediumEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRRadonConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::RadonConcentrationMeasurement::MeasurementUnitEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRRadonConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableRadonConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::RadonConcentrationMeasurement::MeasurementUnitEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableRadonConcentrationMeasurementClusterMeasurementUnitEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelClusterChannelStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Channel::ChannelStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableChannelClusterChannelStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::ChannelStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRChannelClusterLineupInfoTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Channel::LineupInfoTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableChannelClusterLineupInfoTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::LineupInfoTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaPlayback::PlaybackStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableMediaPlaybackClusterPlaybackStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::MediaPlayback::PlaybackStateEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRMediaInputClusterInputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaInput::InputTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableMediaInputClusterInputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::MediaInput::InputTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputClusterCecKeyCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::KeypadInput::CecKeyCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableKeypadInputClusterCecKeyCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::KeypadInput::CecKeyCode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::KeypadInput::KeypadInputStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableKeypadInputClusterKeypadInputStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::KeypadInput::KeypadInputStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterContentLaunchStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterMetricTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::MetricTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterMetricTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ContentLauncher::MetricTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterParameterEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::ParameterEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterParameterEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ContentLauncher::ParameterEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRAudioOutputClusterOutputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AudioOutput::OutputTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableAudioOutputClusterOutputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AudioOutput::OutputTypeEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableApplicationBasicClusterApplicationStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitTestingClusterSimpleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::UnitTesting::SimpleEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRUnitTestingClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableUnitTestingClusterSimpleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::UnitTesting::SimpleEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableUnitTestingClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRFaultInjectionClusterFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FaultInjection::FaultType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableFaultInjectionClusterFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::FaultInjection::FaultType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}
