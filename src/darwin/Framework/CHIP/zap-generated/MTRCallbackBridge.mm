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
#import "MTRStructsObjc.h"

#include <lib/support/TypeTraits.h>

void MTRDefaultSuccessCallbackBridge::OnSuccessFn(void * context) { DispatchSuccess(context, nil); };

void MTRCommandSuccessCallbackBridge::OnSuccessFn(void * context, const chip::app::DataModel::NullObjectType &)
{
    DispatchSuccess(context, nil);
};

void MTROctetStringAttributeCallbackBridge::OnSuccessFn(void * context, chip::ByteSpan value)
{
    NSData * _Nonnull objCValue;
    objCValue = [NSData dataWithBytes:value.data() length:value.size()];
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
        objCValue = [NSData dataWithBytes:value.Value().data() length:value.Value().size()];
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
    objCValue = [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
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
        objCValue = [[NSString alloc] initWithBytes:value.Value().data() length:value.Value().size() encoding:NSUTF8StringEncoding];
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
                        MTRAccessControlClusterTarget * newElement_3;
                        newElement_3 = [MTRAccessControlClusterTarget new];
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
            newElement_0.data = [NSData dataWithBytes:entry_0.data.data() length:entry_0.data.size()];
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
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
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
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
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
            newElement_0 = [[NSString alloc] initWithBytes:entry_0.data() length:entry_0.size() encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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

void MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::TimeFormatLocalization::CalendarType> & value)
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
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::WiredFault> & value)
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
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::BatFault> & value)
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
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::PowerSource::BatChargeFault> & value)
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
    const chip::app::DataModel::DecodableList<chip::app::Clusters::NetworkCommissioning::Structs::NetworkInfo::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRNetworkCommissioningClusterNetworkInfo * newElement_0;
            newElement_0 = [MTRNetworkCommissioningClusterNetworkInfo new];
            newElement_0.networkID = [NSData dataWithBytes:entry_0.networkID.data() length:entry_0.networkID.size()];
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
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
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
            newElement_0.hardwareAddress = [NSData dataWithBytes:entry_0.hardwareAddress.data()
                                                          length:entry_0.hardwareAddress.size()];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = entry_0.IPv4Addresses.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    NSData * newElement_2;
                    newElement_2 = [NSData dataWithBytes:entry_2.data() length:entry_2.size()];
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
                    newElement_2 = [NSData dataWithBytes:entry_2.data() length:entry_2.size()];
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
                newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.Value().data()
                                                             length:entry_0.name.Value().size()
                                                           encoding:NSUTF8StringEncoding];
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
        chip::app::Clusters::ThreadNetworkDiagnostics::Structs::NeighborTable::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRThreadNetworkDiagnosticsClusterNeighborTable * newElement_0;
            newElement_0 = [MTRThreadNetworkDiagnosticsClusterNeighborTable new];
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
    const chip::app::DataModel::DecodableList<chip::app::Clusters::ThreadNetworkDiagnostics::Structs::RouteTable::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRThreadNetworkDiagnosticsClusterRouteTable * newElement_0;
            newElement_0 = [MTRThreadNetworkDiagnosticsClusterRouteTable new];
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

void MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault> & value)
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
            newElement_0.noc = [NSData dataWithBytes:entry_0.noc.data() length:entry_0.noc.size()];
            if (entry_0.icac.IsNull()) {
                newElement_0.icac = nil;
            } else {
                newElement_0.icac = [NSData dataWithBytes:entry_0.icac.Value().data() length:entry_0.icac.Value().size()];
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
            newElement_0.rootPublicKey = [NSData dataWithBytes:entry_0.rootPublicKey.data() length:entry_0.rootPublicKey.size()];
            newElement_0.vendorID = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.vendorID)];
            newElement_0.fabricID = [NSNumber numberWithUnsignedLongLong:entry_0.fabricID];
            newElement_0.nodeID = [NSNumber numberWithUnsignedLongLong:entry_0.nodeID];
            newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                          length:entry_0.label.size()
                                                        encoding:NSUTF8StringEncoding];
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
            newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
            [array_0 addObject:newElement_0];
        }
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
                newElement_0.groupName = [[NSString alloc] initWithBytes:entry_0.groupName.Value().data()
                                                                  length:entry_0.groupName.Value().size()
                                                                encoding:NSUTF8StringEncoding];
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
            newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                          length:entry_0.label.size()
                                                        encoding:NSUTF8StringEncoding];
            newElement_0.value = [[NSString alloc] initWithBytes:entry_0.value.data()
                                                          length:entry_0.value.size()
                                                        encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
            newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                          length:entry_0.label.size()
                                                        encoding:NSUTF8StringEncoding];
            newElement_0.value = [[NSString alloc] initWithBytes:entry_0.value.data()
                                                          length:entry_0.value.size()
                                                        encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
            newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                          length:entry_0.label.size()
                                                        encoding:NSUTF8StringEncoding];
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
    void * context, chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus> value)
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
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Channel::Structs::ChannelInfo::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRChannelClusterChannelInfo * newElement_0;
            newElement_0 = [MTRChannelClusterChannelInfo new];
            newElement_0.majorNumber = [NSNumber numberWithUnsignedShort:entry_0.majorNumber];
            newElement_0.minorNumber = [NSNumber numberWithUnsignedShort:entry_0.minorNumber];
            if (entry_0.name.HasValue()) {
                newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.Value().data()
                                                             length:entry_0.name.Value().size()
                                                           encoding:NSUTF8StringEncoding];
            } else {
                newElement_0.name = nil;
            }
            if (entry_0.callSign.HasValue()) {
                newElement_0.callSign = [[NSString alloc] initWithBytes:entry_0.callSign.Value().data()
                                                                 length:entry_0.callSign.Value().size()
                                                               encoding:NSUTF8StringEncoding];
            } else {
                newElement_0.callSign = nil;
            }
            if (entry_0.affiliateCallSign.HasValue()) {
                newElement_0.affiliateCallSign = [[NSString alloc] initWithBytes:entry_0.affiliateCallSign.Value().data()
                                                                          length:entry_0.affiliateCallSign.Value().size()
                                                                        encoding:NSUTF8StringEncoding];
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

void MTRChannelLineupStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::Structs::LineupInfo::DecodableType> & value)
{
    MTRChannelClusterLineupInfo * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRChannelClusterLineupInfo new];
        objCValue.operatorName = [[NSString alloc] initWithBytes:value.Value().operatorName.data()
                                                          length:value.Value().operatorName.size()
                                                        encoding:NSUTF8StringEncoding];
        if (value.Value().lineupName.HasValue()) {
            objCValue.lineupName = [[NSString alloc] initWithBytes:value.Value().lineupName.Value().data()
                                                            length:value.Value().lineupName.Value().size()
                                                          encoding:NSUTF8StringEncoding];
        } else {
            objCValue.lineupName = nil;
        }
        if (value.Value().postalCode.HasValue()) {
            objCValue.postalCode = [[NSString alloc] initWithBytes:value.Value().postalCode.Value().data()
                                                            length:value.Value().postalCode.Value().size()
                                                          encoding:NSUTF8StringEncoding];
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

void MTRChannelCurrentChannelStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Channel::Structs::ChannelInfo::DecodableType> & value)
{
    MTRChannelClusterChannelInfo * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRChannelClusterChannelInfo new];
        objCValue.majorNumber = [NSNumber numberWithUnsignedShort:value.Value().majorNumber];
        objCValue.minorNumber = [NSNumber numberWithUnsignedShort:value.Value().minorNumber];
        if (value.Value().name.HasValue()) {
            objCValue.name = [[NSString alloc] initWithBytes:value.Value().name.Value().data()
                                                      length:value.Value().name.Value().size()
                                                    encoding:NSUTF8StringEncoding];
        } else {
            objCValue.name = nil;
        }
        if (value.Value().callSign.HasValue()) {
            objCValue.callSign = [[NSString alloc] initWithBytes:value.Value().callSign.Value().data()
                                                          length:value.Value().callSign.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            objCValue.callSign = nil;
        }
        if (value.Value().affiliateCallSign.HasValue()) {
            objCValue.affiliateCallSign = [[NSString alloc] initWithBytes:value.Value().affiliateCallSign.Value().data()
                                                                   length:value.Value().affiliateCallSign.Value().size()
                                                                 encoding:NSUTF8StringEncoding];
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
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
    const chip::app::DataModel::Nullable<chip::app::Clusters::MediaPlayback::Structs::PlaybackPosition::DecodableType> & value)
{
    MTRMediaPlaybackClusterPlaybackPosition * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRMediaPlaybackClusterPlaybackPosition new];
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
    const chip::app::DataModel::DecodableList<chip::app::Clusters::MediaInput::Structs::InputInfo::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRMediaInputClusterInputInfo * newElement_0;
            newElement_0 = [MTRMediaInputClusterInputInfo new];
            newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
            newElement_0.inputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.inputType)];
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
            newElement_0.descriptionString = [[NSString alloc] initWithBytes:entry_0.description.data()
                                                                      length:entry_0.description.size()
                                                                    encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
            newElement_0 = [[NSString alloc] initWithBytes:entry_0.data() length:entry_0.size() encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
    const chip::app::DataModel::DecodableList<chip::app::Clusters::AudioOutput::Structs::OutputInfo::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAudioOutputClusterOutputInfo * newElement_0;
            newElement_0 = [MTRAudioOutputClusterOutputInfo new];
            newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
            newElement_0.outputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.outputType)];
            newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                         length:entry_0.name.size()
                                                       encoding:NSUTF8StringEncoding];
            [array_0 addObject:newElement_0];
        }
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
    const chip::app::DataModel::Nullable<chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEP::DecodableType> & value)
{
    MTRApplicationLauncherClusterApplicationEP * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRApplicationLauncherClusterApplicationEP new];
        objCValue.application = [MTRApplicationLauncherClusterApplication new];
        objCValue.application.catalogVendorId = [NSNumber numberWithUnsignedShort:value.Value().application.catalogVendorId];
        objCValue.application.applicationId = [[NSString alloc] initWithBytes:value.Value().application.applicationId.data()
                                                                       length:value.Value().application.applicationId.size()
                                                                     encoding:NSUTF8StringEncoding];
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
    void * context, const chip::app::Clusters::ApplicationBasic::Structs::ApplicationBasicApplication::DecodableType & value)
{
    MTRApplicationBasicClusterApplicationBasicApplication * _Nonnull objCValue;
    objCValue = [MTRApplicationBasicClusterApplicationBasicApplication new];
    objCValue.catalogVendorId = [NSNumber numberWithUnsignedShort:value.catalogVendorId];
    objCValue.applicationId = [[NSString alloc] initWithBytes:value.applicationId.data()
                                                       length:value.applicationId.size()
                                                     encoding:NSUTF8StringEncoding];
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
            newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
            [array_0 addObject:newElement_0];
        }
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
            newElement_0.member2 = [NSData dataWithBytes:entry_0.member2.data() length:entry_0.member2.size()];
            [array_0 addObject:newElement_0];
        }
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
                newElement_0.nullableString = [[NSString alloc] initWithBytes:entry_0.nullableString.Value().data()
                                                                       length:entry_0.nullableString.Value().size()
                                                                     encoding:NSUTF8StringEncoding];
            }
            if (entry_0.optionalString.HasValue()) {
                newElement_0.optionalString = [[NSString alloc] initWithBytes:entry_0.optionalString.Value().data()
                                                                       length:entry_0.optionalString.Value().size()
                                                                     encoding:NSUTF8StringEncoding];
            } else {
                newElement_0.optionalString = nil;
            }
            if (entry_0.nullableOptionalString.HasValue()) {
                if (entry_0.nullableOptionalString.Value().IsNull()) {
                    newElement_0.nullableOptionalString = nil;
                } else {
                    newElement_0.nullableOptionalString =
                        [[NSString alloc] initWithBytes:entry_0.nullableOptionalString.Value().Value().data()
                                                 length:entry_0.nullableOptionalString.Value().Value().size()
                                               encoding:NSUTF8StringEncoding];
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
                newElement_0.nullableStruct.d = [NSData dataWithBytes:entry_0.nullableStruct.Value().d.data()
                                                               length:entry_0.nullableStruct.Value().d.size()];
                newElement_0.nullableStruct.e = [[NSString alloc] initWithBytes:entry_0.nullableStruct.Value().e.data()
                                                                         length:entry_0.nullableStruct.Value().e.size()
                                                                       encoding:NSUTF8StringEncoding];
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
                newElement_0.optionalStruct.d = [NSData dataWithBytes:entry_0.optionalStruct.Value().d.data()
                                                               length:entry_0.optionalStruct.Value().d.size()];
                newElement_0.optionalStruct.e = [[NSString alloc] initWithBytes:entry_0.optionalStruct.Value().e.data()
                                                                         length:entry_0.optionalStruct.Value().e.size()
                                                                       encoding:NSUTF8StringEncoding];
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
                    newElement_0.nullableOptionalStruct.d =
                        [NSData dataWithBytes:entry_0.nullableOptionalStruct.Value().Value().d.data()
                                       length:entry_0.nullableOptionalStruct.Value().Value().d.size()];
                    newElement_0.nullableOptionalStruct.e =
                        [[NSString alloc] initWithBytes:entry_0.nullableOptionalStruct.Value().Value().e.data()
                                                 length:entry_0.nullableOptionalStruct.Value().Value().e.size()
                                               encoding:NSUTF8StringEncoding];
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
    objCValue.d = [NSData dataWithBytes:value.d.data() length:value.d.size()];
    objCValue.e = [[NSString alloc] initWithBytes:value.e.data() length:value.e.size() encoding:NSUTF8StringEncoding];
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
            newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
            [array_0 addObject:newElement_0];
        }
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
            newElement_0.fabricSensitiveCharString = [[NSString alloc] initWithBytes:entry_0.fabricSensitiveCharString.data()
                                                                              length:entry_0.fabricSensitiveCharString.size()
                                                                            encoding:NSUTF8StringEncoding];
            newElement_0.fabricSensitiveStruct = [MTRUnitTestingClusterSimpleStruct new];
            newElement_0.fabricSensitiveStruct.a = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.a];
            newElement_0.fabricSensitiveStruct.b = [NSNumber numberWithBool:entry_0.fabricSensitiveStruct.b];
            newElement_0.fabricSensitiveStruct.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.fabricSensitiveStruct.c)];
            newElement_0.fabricSensitiveStruct.d = [NSData dataWithBytes:entry_0.fabricSensitiveStruct.d.data()
                                                                  length:entry_0.fabricSensitiveStruct.d.size()];
            newElement_0.fabricSensitiveStruct.e = [[NSString alloc] initWithBytes:entry_0.fabricSensitiveStruct.e.data()
                                                                            length:entry_0.fabricSensitiveStruct.e.size()
                                                                          encoding:NSUTF8StringEncoding];
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
        objCValue.d = [NSData dataWithBytes:value.Value().d.data() length:value.Value().d.size()];
        objCValue.e = [[NSString alloc] initWithBytes:value.Value().e.data()
                                               length:value.Value().e.size()
                                             encoding:NSUTF8StringEncoding];
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

void MTRGroupsClusterAddGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterAddGroupResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupID = [NSNumber numberWithUnsignedShort:data.groupID];
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterViewGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterViewGroupResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupID = [NSNumber numberWithUnsignedShort:data.groupID];
    }
    {
        response.groupName = [[NSString alloc] initWithBytes:data.groupName.data()
                                                      length:data.groupName.size()
                                                    encoding:NSUTF8StringEncoding];
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterGetGroupMembershipResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterGetGroupMembershipResponseParams new];
    {
        if (data.capacity.IsNull()) {
            response.capacity = nil;
        } else {
            response.capacity = [NSNumber numberWithUnsignedChar:data.capacity.Value()];
        }
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = data.groupList.begin();
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
            response.groupList = array_0;
        }
    }
    DispatchSuccess(context, response);
};

void MTRGroupsClusterRemoveGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterRemoveGroupResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupID = [NSNumber numberWithUnsignedShort:data.groupID];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterAddSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterAddSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterViewSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterViewSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    {
        if (data.transitionTime.HasValue()) {
            response.transitionTime = [NSNumber numberWithUnsignedShort:data.transitionTime.Value()];
        } else {
            response.transitionTime = nil;
        }
    }
    {
        if (data.sceneName.HasValue()) {
            response.sceneName = [[NSString alloc] initWithBytes:data.sceneName.Value().data()
                                                          length:data.sceneName.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.sceneName = nil;
        }
    }
    {
        if (data.extensionFieldSets.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.extensionFieldSets.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRScenesClusterExtensionFieldSet * newElement_1;
                    newElement_1 = [MTRScenesClusterExtensionFieldSet new];
                    newElement_1.clusterId = [NSNumber numberWithUnsignedInt:entry_1.clusterId];
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_1.attributeValueList.begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            MTRScenesClusterAttributeValuePair * newElement_3;
                            newElement_3 = [MTRScenesClusterAttributeValuePair new];
                            if (entry_3.attributeId.HasValue()) {
                                newElement_3.attributeId = [NSNumber numberWithUnsignedInt:entry_3.attributeId.Value()];
                            } else {
                                newElement_3.attributeId = nil;
                            }
                            { // Scope for our temporary variables
                                auto * array_5 = [NSMutableArray new];
                                auto iter_5 = entry_3.attributeValue.begin();
                                while (iter_5.Next()) {
                                    auto & entry_5 = iter_5.GetValue();
                                    NSNumber * newElement_5;
                                    newElement_5 = [NSNumber numberWithUnsignedChar:entry_5];
                                    [array_5 addObject:newElement_5];
                                }
                                CHIP_ERROR err = iter_5.GetStatus();
                                if (err != CHIP_NO_ERROR) {
                                    OnFailureFn(context, err);
                                    return;
                                }
                                newElement_3.attributeValue = array_5;
                            }
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            OnFailureFn(context, err);
                            return;
                        }
                        newElement_1.attributeValueList = array_3;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.extensionFieldSets = array_1;
            }
        } else {
            response.extensionFieldSets = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterRemoveSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterRemoveSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterRemoveAllScenesResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterRemoveAllScenesResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterStoreSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterStoreSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterGetSceneMembershipResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterGetSceneMembershipResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        if (data.capacity.IsNull()) {
            response.capacity = nil;
        } else {
            response.capacity = [NSNumber numberWithUnsignedChar:data.capacity.Value()];
        }
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        if (data.sceneList.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.sceneList.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:entry_1];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.sceneList = array_1;
            }
        } else {
            response.sceneList = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterEnhancedAddSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterEnhancedAddSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterEnhancedViewSceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterEnhancedViewSceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
    }
    {
        response.sceneId = [NSNumber numberWithUnsignedChar:data.sceneId];
    }
    {
        if (data.transitionTime.HasValue()) {
            response.transitionTime = [NSNumber numberWithUnsignedShort:data.transitionTime.Value()];
        } else {
            response.transitionTime = nil;
        }
    }
    {
        if (data.sceneName.HasValue()) {
            response.sceneName = [[NSString alloc] initWithBytes:data.sceneName.Value().data()
                                                          length:data.sceneName.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.sceneName = nil;
        }
    }
    {
        if (data.extensionFieldSets.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.extensionFieldSets.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRScenesClusterExtensionFieldSet * newElement_1;
                    newElement_1 = [MTRScenesClusterExtensionFieldSet new];
                    newElement_1.clusterId = [NSNumber numberWithUnsignedInt:entry_1.clusterId];
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_1.attributeValueList.begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            MTRScenesClusterAttributeValuePair * newElement_3;
                            newElement_3 = [MTRScenesClusterAttributeValuePair new];
                            if (entry_3.attributeId.HasValue()) {
                                newElement_3.attributeId = [NSNumber numberWithUnsignedInt:entry_3.attributeId.Value()];
                            } else {
                                newElement_3.attributeId = nil;
                            }
                            { // Scope for our temporary variables
                                auto * array_5 = [NSMutableArray new];
                                auto iter_5 = entry_3.attributeValue.begin();
                                while (iter_5.Next()) {
                                    auto & entry_5 = iter_5.GetValue();
                                    NSNumber * newElement_5;
                                    newElement_5 = [NSNumber numberWithUnsignedChar:entry_5];
                                    [array_5 addObject:newElement_5];
                                }
                                CHIP_ERROR err = iter_5.GetStatus();
                                if (err != CHIP_NO_ERROR) {
                                    OnFailureFn(context, err);
                                    return;
                                }
                                newElement_3.attributeValue = array_5;
                            }
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            OnFailureFn(context, err);
                            return;
                        }
                        newElement_1.attributeValueList = array_3;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.extensionFieldSets = array_1;
            }
        } else {
            response.extensionFieldSets = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRScenesClusterCopySceneResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType & data)
{
    auto * response = [MTRScenesClusterCopySceneResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupIdFrom = [NSNumber numberWithUnsignedShort:data.groupIdFrom];
    }
    {
        response.sceneIdFrom = [NSNumber numberWithUnsignedChar:data.sceneIdFrom];
    }
    DispatchSuccess(context, response);
};

void MTROTASoftwareUpdateProviderClusterQueryImageResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType & data)
{
    auto * response = [MTROTASoftwareUpdateProviderClusterQueryImageResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.delayedActionTime.HasValue()) {
            response.delayedActionTime = [NSNumber numberWithUnsignedInt:data.delayedActionTime.Value()];
        } else {
            response.delayedActionTime = nil;
        }
    }
    {
        if (data.imageURI.HasValue()) {
            response.imageURI = [[NSString alloc] initWithBytes:data.imageURI.Value().data()
                                                         length:data.imageURI.Value().size()
                                                       encoding:NSUTF8StringEncoding];
        } else {
            response.imageURI = nil;
        }
    }
    {
        if (data.softwareVersion.HasValue()) {
            response.softwareVersion = [NSNumber numberWithUnsignedInt:data.softwareVersion.Value()];
        } else {
            response.softwareVersion = nil;
        }
    }
    {
        if (data.softwareVersionString.HasValue()) {
            response.softwareVersionString = [[NSString alloc] initWithBytes:data.softwareVersionString.Value().data()
                                                                      length:data.softwareVersionString.Value().size()
                                                                    encoding:NSUTF8StringEncoding];
        } else {
            response.softwareVersionString = nil;
        }
    }
    {
        if (data.updateToken.HasValue()) {
            response.updateToken = [NSData dataWithBytes:data.updateToken.Value().data() length:data.updateToken.Value().size()];
        } else {
            response.updateToken = nil;
        }
    }
    {
        if (data.userConsentNeeded.HasValue()) {
            response.userConsentNeeded = [NSNumber numberWithBool:data.userConsentNeeded.Value()];
        } else {
            response.userConsentNeeded = nil;
        }
    }
    {
        if (data.metadataForRequestor.HasValue()) {
            response.metadataForRequestor = [NSData dataWithBytes:data.metadataForRequestor.Value().data()
                                                           length:data.metadataForRequestor.Value().size()];
        } else {
            response.metadataForRequestor = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTROTASoftwareUpdateProviderClusterApplyUpdateResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType & data)
{
    auto * response = [MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams new];
    {
        response.action = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.action)];
    }
    {
        response.delayedActionTime = [NSNumber numberWithUnsignedInt:data.delayedActionTime];
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterArmFailSafeResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterArmFailSafeResponseParams new];
    {
        response.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.errorCode)];
    }
    {
        response.debugText = [[NSString alloc] initWithBytes:data.debugText.data()
                                                      length:data.debugText.size()
                                                    encoding:NSUTF8StringEncoding];
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterSetRegulatoryConfigResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams new];
    {
        response.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.errorCode)];
    }
    {
        response.debugText = [[NSString alloc] initWithBytes:data.debugText.data()
                                                      length:data.debugText.size()
                                                    encoding:NSUTF8StringEncoding];
    }
    DispatchSuccess(context, response);
};

void MTRGeneralCommissioningClusterCommissioningCompleteResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    auto * response = [MTRGeneralCommissioningClusterCommissioningCompleteResponseParams new];
    {
        response.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.errorCode)];
    }
    {
        response.debugText = [[NSString alloc] initWithBytes:data.debugText.data()
                                                      length:data.debugText.size()
                                                    encoding:NSUTF8StringEncoding];
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterScanNetworksResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterScanNetworksResponseParams new];
    {
        response.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.networkingStatus)];
    }
    {
        if (data.debugText.HasValue()) {
            response.debugText = [[NSString alloc] initWithBytes:data.debugText.Value().data()
                                                          length:data.debugText.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.debugText = nil;
        }
    }
    {
        if (data.wiFiScanResults.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.wiFiScanResults.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRNetworkCommissioningClusterWiFiInterfaceScanResult * newElement_1;
                    newElement_1 = [MTRNetworkCommissioningClusterWiFiInterfaceScanResult new];
                    newElement_1.security = [NSNumber numberWithUnsignedChar:entry_1.security.Raw()];
                    newElement_1.ssid = [NSData dataWithBytes:entry_1.ssid.data() length:entry_1.ssid.size()];
                    newElement_1.bssid = [NSData dataWithBytes:entry_1.bssid.data() length:entry_1.bssid.size()];
                    newElement_1.channel = [NSNumber numberWithUnsignedShort:entry_1.channel];
                    newElement_1.wiFiBand = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1.wiFiBand)];
                    newElement_1.rssi = [NSNumber numberWithChar:entry_1.rssi];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.wiFiScanResults = array_1;
            }
        } else {
            response.wiFiScanResults = nil;
        }
    }
    {
        if (data.threadScanResults.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.threadScanResults.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRNetworkCommissioningClusterThreadInterfaceScanResult * newElement_1;
                    newElement_1 = [MTRNetworkCommissioningClusterThreadInterfaceScanResult new];
                    newElement_1.panId = [NSNumber numberWithUnsignedShort:entry_1.panId];
                    newElement_1.extendedPanId = [NSNumber numberWithUnsignedLongLong:entry_1.extendedPanId];
                    newElement_1.networkName = [[NSString alloc] initWithBytes:entry_1.networkName.data()
                                                                        length:entry_1.networkName.size()
                                                                      encoding:NSUTF8StringEncoding];
                    newElement_1.channel = [NSNumber numberWithUnsignedShort:entry_1.channel];
                    newElement_1.version = [NSNumber numberWithUnsignedChar:entry_1.version];
                    newElement_1.extendedAddress = [NSData dataWithBytes:entry_1.extendedAddress.data()
                                                                  length:entry_1.extendedAddress.size()];
                    newElement_1.rssi = [NSNumber numberWithChar:entry_1.rssi];
                    newElement_1.lqi = [NSNumber numberWithUnsignedChar:entry_1.lqi];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.threadScanResults = array_1;
            }
        } else {
            response.threadScanResults = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterNetworkConfigResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterNetworkConfigResponseParams new];
    {
        response.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.networkingStatus)];
    }
    {
        if (data.debugText.HasValue()) {
            response.debugText = [[NSString alloc] initWithBytes:data.debugText.Value().data()
                                                          length:data.debugText.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.debugText = nil;
        }
    }
    {
        if (data.networkIndex.HasValue()) {
            response.networkIndex = [NSNumber numberWithUnsignedChar:data.networkIndex.Value()];
        } else {
            response.networkIndex = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRNetworkCommissioningClusterConnectNetworkResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
{
    auto * response = [MTRNetworkCommissioningClusterConnectNetworkResponseParams new];
    {
        response.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.networkingStatus)];
    }
    {
        if (data.debugText.HasValue()) {
            response.debugText = [[NSString alloc] initWithBytes:data.debugText.Value().data()
                                                          length:data.debugText.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.debugText = nil;
        }
    }
    {
        if (data.errorValue.IsNull()) {
            response.errorValue = nil;
        } else {
            response.errorValue = [NSNumber numberWithInt:data.errorValue.Value()];
        }
    }
    DispatchSuccess(context, response);
};

void MTRDiagnosticLogsClusterRetrieveLogsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType & data)
{
    auto * response = [MTRDiagnosticLogsClusterRetrieveLogsResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        response.logContent = [NSData dataWithBytes:data.logContent.data() length:data.logContent.size()];
    }
    {
        response.utcTimeStamp = [NSNumber numberWithUnsignedInt:data.UTCTimeStamp];
    }
    {
        response.timeSinceBoot = [NSNumber numberWithUnsignedInt:data.timeSinceBoot];
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterAttestationResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterAttestationResponseParams new];
    {
        response.attestationElements = [NSData dataWithBytes:data.attestationElements.data()
                                                      length:data.attestationElements.size()];
    }
    {
        response.attestationSignature = [NSData dataWithBytes:data.attestationSignature.data()
                                                       length:data.attestationSignature.size()];
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterCertificateChainResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterCertificateChainResponseParams new];
    {
        response.certificate = [NSData dataWithBytes:data.certificate.data() length:data.certificate.size()];
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterCSRResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterCSRResponseParams new];
    {
        response.nocsrElements = [NSData dataWithBytes:data.NOCSRElements.data() length:data.NOCSRElements.size()];
    }
    {
        response.attestationSignature = [NSData dataWithBytes:data.attestationSignature.data()
                                                       length:data.attestationSignature.size()];
    }
    DispatchSuccess(context, response);
};

void MTROperationalCredentialsClusterNOCResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    auto * response = [MTROperationalCredentialsClusterNOCResponseParams new];
    {
        response.statusCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.statusCode)];
    }
    {
        if (data.fabricIndex.HasValue()) {
            response.fabricIndex = [NSNumber numberWithUnsignedChar:data.fabricIndex.Value()];
        } else {
            response.fabricIndex = nil;
        }
    }
    {
        if (data.debugText.HasValue()) {
            response.debugText = [[NSString alloc] initWithBytes:data.debugText.Value().data()
                                                          length:data.debugText.Value().size()
                                                        encoding:NSUTF8StringEncoding];
        } else {
            response.debugText = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRGroupKeyManagementClusterKeySetReadResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType & data)
{
    auto * response = [MTRGroupKeyManagementClusterKeySetReadResponseParams new];
    {
        response.groupKeySet = [MTRGroupKeyManagementClusterGroupKeySetStruct new];
        response.groupKeySet.groupKeySetID = [NSNumber numberWithUnsignedShort:data.groupKeySet.groupKeySetID];
        response.groupKeySet.groupKeySecurityPolicy =
            [NSNumber numberWithUnsignedChar:chip::to_underlying(data.groupKeySet.groupKeySecurityPolicy)];
        if (data.groupKeySet.epochKey0.IsNull()) {
            response.groupKeySet.epochKey0 = nil;
        } else {
            response.groupKeySet.epochKey0 = [NSData dataWithBytes:data.groupKeySet.epochKey0.Value().data()
                                                            length:data.groupKeySet.epochKey0.Value().size()];
        }
        if (data.groupKeySet.epochStartTime0.IsNull()) {
            response.groupKeySet.epochStartTime0 = nil;
        } else {
            response.groupKeySet.epochStartTime0 = [NSNumber numberWithUnsignedLongLong:data.groupKeySet.epochStartTime0.Value()];
        }
        if (data.groupKeySet.epochKey1.IsNull()) {
            response.groupKeySet.epochKey1 = nil;
        } else {
            response.groupKeySet.epochKey1 = [NSData dataWithBytes:data.groupKeySet.epochKey1.Value().data()
                                                            length:data.groupKeySet.epochKey1.Value().size()];
        }
        if (data.groupKeySet.epochStartTime1.IsNull()) {
            response.groupKeySet.epochStartTime1 = nil;
        } else {
            response.groupKeySet.epochStartTime1 = [NSNumber numberWithUnsignedLongLong:data.groupKeySet.epochStartTime1.Value()];
        }
        if (data.groupKeySet.epochKey2.IsNull()) {
            response.groupKeySet.epochKey2 = nil;
        } else {
            response.groupKeySet.epochKey2 = [NSData dataWithBytes:data.groupKeySet.epochKey2.Value().data()
                                                            length:data.groupKeySet.epochKey2.Value().size()];
        }
        if (data.groupKeySet.epochStartTime2.IsNull()) {
            response.groupKeySet.epochStartTime2 = nil;
        } else {
            response.groupKeySet.epochStartTime2 = [NSNumber numberWithUnsignedLongLong:data.groupKeySet.epochStartTime2.Value()];
        }
    }
    DispatchSuccess(context, response);
};

void MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType & data)
{
    auto * response = [MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams new];
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = data.groupKeySetIDs.begin();
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
            response.groupKeySetIDs = array_0;
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetWeekDayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetWeekDayScheduleResponseParams new];
    {
        response.weekDayIndex = [NSNumber numberWithUnsignedChar:data.weekDayIndex];
    }
    {
        response.userIndex = [NSNumber numberWithUnsignedShort:data.userIndex];
    }
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.daysMask.HasValue()) {
            response.daysMask = [NSNumber numberWithUnsignedChar:data.daysMask.Value().Raw()];
        } else {
            response.daysMask = nil;
        }
    }
    {
        if (data.startHour.HasValue()) {
            response.startHour = [NSNumber numberWithUnsignedChar:data.startHour.Value()];
        } else {
            response.startHour = nil;
        }
    }
    {
        if (data.startMinute.HasValue()) {
            response.startMinute = [NSNumber numberWithUnsignedChar:data.startMinute.Value()];
        } else {
            response.startMinute = nil;
        }
    }
    {
        if (data.endHour.HasValue()) {
            response.endHour = [NSNumber numberWithUnsignedChar:data.endHour.Value()];
        } else {
            response.endHour = nil;
        }
    }
    {
        if (data.endMinute.HasValue()) {
            response.endMinute = [NSNumber numberWithUnsignedChar:data.endMinute.Value()];
        } else {
            response.endMinute = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetYearDayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetYearDayScheduleResponseParams new];
    {
        response.yearDayIndex = [NSNumber numberWithUnsignedChar:data.yearDayIndex];
    }
    {
        response.userIndex = [NSNumber numberWithUnsignedShort:data.userIndex];
    }
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.localStartTime.HasValue()) {
            response.localStartTime = [NSNumber numberWithUnsignedInt:data.localStartTime.Value()];
        } else {
            response.localStartTime = nil;
        }
    }
    {
        if (data.localEndTime.HasValue()) {
            response.localEndTime = [NSNumber numberWithUnsignedInt:data.localEndTime.Value()];
        } else {
            response.localEndTime = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetHolidayScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetHolidayScheduleResponseParams new];
    {
        response.holidayIndex = [NSNumber numberWithUnsignedChar:data.holidayIndex];
    }
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.localStartTime.HasValue()) {
            response.localStartTime = [NSNumber numberWithUnsignedInt:data.localStartTime.Value()];
        } else {
            response.localStartTime = nil;
        }
    }
    {
        if (data.localEndTime.HasValue()) {
            response.localEndTime = [NSNumber numberWithUnsignedInt:data.localEndTime.Value()];
        } else {
            response.localEndTime = nil;
        }
    }
    {
        if (data.operatingMode.HasValue()) {
            response.operatingMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.operatingMode.Value())];
        } else {
            response.operatingMode = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetUserResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetUserResponseParams new];
    {
        response.userIndex = [NSNumber numberWithUnsignedShort:data.userIndex];
    }
    {
        if (data.userName.IsNull()) {
            response.userName = nil;
        } else {
            response.userName = [[NSString alloc] initWithBytes:data.userName.Value().data()
                                                         length:data.userName.Value().size()
                                                       encoding:NSUTF8StringEncoding];
        }
    }
    {
        if (data.userUniqueID.IsNull()) {
            response.userUniqueID = nil;
        } else {
            response.userUniqueID = [NSNumber numberWithUnsignedInt:data.userUniqueID.Value()];
        }
    }
    {
        if (data.userStatus.IsNull()) {
            response.userStatus = nil;
        } else {
            response.userStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.userStatus.Value())];
        }
    }
    {
        if (data.userType.IsNull()) {
            response.userType = nil;
        } else {
            response.userType = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.userType.Value())];
        }
    }
    {
        if (data.credentialRule.IsNull()) {
            response.credentialRule = nil;
        } else {
            response.credentialRule = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.credentialRule.Value())];
        }
    }
    {
        if (data.credentials.IsNull()) {
            response.credentials = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.credentials.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRDoorLockClusterCredentialStruct * newElement_1;
                    newElement_1 = [MTRDoorLockClusterCredentialStruct new];
                    newElement_1.credentialType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1.credentialType)];
                    newElement_1.credentialIndex = [NSNumber numberWithUnsignedShort:entry_1.credentialIndex];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.credentials = array_1;
            }
        }
    }
    {
        if (data.creatorFabricIndex.IsNull()) {
            response.creatorFabricIndex = nil;
        } else {
            response.creatorFabricIndex = [NSNumber numberWithUnsignedChar:data.creatorFabricIndex.Value()];
        }
    }
    {
        if (data.lastModifiedFabricIndex.IsNull()) {
            response.lastModifiedFabricIndex = nil;
        } else {
            response.lastModifiedFabricIndex = [NSNumber numberWithUnsignedChar:data.lastModifiedFabricIndex.Value()];
        }
    }
    {
        if (data.nextUserIndex.IsNull()) {
            response.nextUserIndex = nil;
        } else {
            response.nextUserIndex = [NSNumber numberWithUnsignedShort:data.nextUserIndex.Value()];
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterSetCredentialResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterSetCredentialResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.userIndex.IsNull()) {
            response.userIndex = nil;
        } else {
            response.userIndex = [NSNumber numberWithUnsignedShort:data.userIndex.Value()];
        }
    }
    {
        if (data.nextCredentialIndex.IsNull()) {
            response.nextCredentialIndex = nil;
        } else {
            response.nextCredentialIndex = [NSNumber numberWithUnsignedShort:data.nextCredentialIndex.Value()];
        }
    }
    DispatchSuccess(context, response);
};

void MTRDoorLockClusterGetCredentialStatusResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType & data)
{
    auto * response = [MTRDoorLockClusterGetCredentialStatusResponseParams new];
    {
        response.credentialExists = [NSNumber numberWithBool:data.credentialExists];
    }
    {
        if (data.userIndex.IsNull()) {
            response.userIndex = nil;
        } else {
            response.userIndex = [NSNumber numberWithUnsignedShort:data.userIndex.Value()];
        }
    }
    {
        if (data.creatorFabricIndex.IsNull()) {
            response.creatorFabricIndex = nil;
        } else {
            response.creatorFabricIndex = [NSNumber numberWithUnsignedChar:data.creatorFabricIndex.Value()];
        }
    }
    {
        if (data.lastModifiedFabricIndex.IsNull()) {
            response.lastModifiedFabricIndex = nil;
        } else {
            response.lastModifiedFabricIndex = [NSNumber numberWithUnsignedChar:data.lastModifiedFabricIndex.Value()];
        }
    }
    {
        if (data.nextCredentialIndex.IsNull()) {
            response.nextCredentialIndex = nil;
        } else {
            response.nextCredentialIndex = [NSNumber numberWithUnsignedShort:data.nextCredentialIndex.Value()];
        }
    }
    DispatchSuccess(context, response);
};

void MTRThermostatClusterGetWeeklyScheduleResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType & data)
{
    auto * response = [MTRThermostatClusterGetWeeklyScheduleResponseParams new];
    {
        response.numberOfTransitionsForSequence = [NSNumber numberWithUnsignedChar:data.numberOfTransitionsForSequence];
    }
    {
        response.dayOfWeekForSequence = [NSNumber numberWithUnsignedChar:data.dayOfWeekForSequence.Raw()];
    }
    {
        response.modeForSequence = [NSNumber numberWithUnsignedChar:data.modeForSequence.Raw()];
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = data.transitions.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                MTRThermostatClusterThermostatScheduleTransition * newElement_0;
                newElement_0 = [MTRThermostatClusterThermostatScheduleTransition new];
                newElement_0.transitionTime = [NSNumber numberWithUnsignedShort:entry_0.transitionTime];
                if (entry_0.heatSetpoint.IsNull()) {
                    newElement_0.heatSetpoint = nil;
                } else {
                    newElement_0.heatSetpoint = [NSNumber numberWithShort:entry_0.heatSetpoint.Value()];
                }
                if (entry_0.coolSetpoint.IsNull()) {
                    newElement_0.coolSetpoint = nil;
                } else {
                    newElement_0.coolSetpoint = [NSNumber numberWithShort:entry_0.coolSetpoint.Value()];
                }
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                OnFailureFn(context, err);
                return;
            }
            response.transitions = array_0;
        }
    }
    DispatchSuccess(context, response);
};

void MTRChannelClusterChangeChannelResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType & data)
{
    auto * response = [MTRChannelClusterChangeChannelResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.data.HasValue()) {
            response.data = [[NSString alloc] initWithBytes:data.data.Value().data()
                                                     length:data.data.Value().size()
                                                   encoding:NSUTF8StringEncoding];
        } else {
            response.data = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRTargetNavigatorClusterNavigateTargetResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType & data)
{
    auto * response = [MTRTargetNavigatorClusterNavigateTargetResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.data.HasValue()) {
            response.data = [[NSString alloc] initWithBytes:data.data.Value().data()
                                                     length:data.data.Value().size()
                                                   encoding:NSUTF8StringEncoding];
        } else {
            response.data = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRMediaPlaybackClusterPlaybackResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType & data)
{
    auto * response = [MTRMediaPlaybackClusterPlaybackResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.data.HasValue()) {
            response.data = [[NSString alloc] initWithBytes:data.data.Value().data()
                                                     length:data.data.Value().size()
                                                   encoding:NSUTF8StringEncoding];
        } else {
            response.data = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRKeypadInputClusterSendKeyResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType & data)
{
    auto * response = [MTRKeypadInputClusterSendKeyResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    DispatchSuccess(context, response);
};

void MTRContentLauncherClusterLaunchResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::DecodableType & data)
{
    auto * response = [MTRContentLauncherClusterLaunchResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        if (data.data.HasValue()) {
            response.data = [[NSString alloc] initWithBytes:data.data.Value().data()
                                                     length:data.data.Value().size()
                                                   encoding:NSUTF8StringEncoding];
        } else {
            response.data = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRApplicationLauncherClusterLauncherResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType & data)
{
    auto * response = [MTRApplicationLauncherClusterLauncherResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.status)];
    }
    {
        response.data = [NSData dataWithBytes:data.data.data() length:data.data.size()];
    }
    DispatchSuccess(context, response);
};

void MTRAccountLoginClusterGetSetupPINResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType & data)
{
    auto * response = [MTRAccountLoginClusterGetSetupPINResponseParams new];
    {
        response.setupPIN = [[NSString alloc] initWithBytes:data.setupPIN.data()
                                                     length:data.setupPIN.size()
                                                   encoding:NSUTF8StringEncoding];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestSpecificResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestSpecificResponseParams new];
    {
        response.returnValue = [NSNumber numberWithUnsignedChar:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestAddArgumentsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestAddArgumentsResponseParams new];
    {
        response.returnValue = [NSNumber numberWithUnsignedChar:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestSimpleArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestSimpleArgumentResponseParams new];
    {
        response.returnValue = [NSNumber numberWithBool:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestStructArrayArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestStructArrayArgumentResponseParams new];
    { { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
    auto iter_0 = data.arg1.begin();
    while (iter_0.Next()) {
        auto & entry_0 = iter_0.GetValue();
        MTRUnitTestingClusterNestedStructList * newElement_0;
        newElement_0 = [MTRUnitTestingClusterNestedStructList new];
        newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
        newElement_0.b = [NSNumber numberWithBool:entry_0.b];
        newElement_0.c = [MTRUnitTestingClusterSimpleStruct new];
        newElement_0.c.a = [NSNumber numberWithUnsignedChar:entry_0.c.a];
        newElement_0.c.b = [NSNumber numberWithBool:entry_0.c.b];
        newElement_0.c.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.c.c)];
        newElement_0.c.d = [NSData dataWithBytes:entry_0.c.d.data() length:entry_0.c.d.size()];
        newElement_0.c.e = [[NSString alloc] initWithBytes:entry_0.c.e.data()
                                                    length:entry_0.c.e.size()
                                                  encoding:NSUTF8StringEncoding];
        newElement_0.c.f = [NSNumber numberWithUnsignedChar:entry_0.c.f.Raw()];
        newElement_0.c.g = [NSNumber numberWithFloat:entry_0.c.g];
        newElement_0.c.h = [NSNumber numberWithDouble:entry_0.c.h];
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.d.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                MTRUnitTestingClusterSimpleStruct * newElement_2;
                newElement_2 = [MTRUnitTestingClusterSimpleStruct new];
                newElement_2.a = [NSNumber numberWithUnsignedChar:entry_2.a];
                newElement_2.b = [NSNumber numberWithBool:entry_2.b];
                newElement_2.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.c)];
                newElement_2.d = [NSData dataWithBytes:entry_2.d.data() length:entry_2.d.size()];
                newElement_2.e = [[NSString alloc] initWithBytes:entry_2.e.data()
                                                          length:entry_2.e.size()
                                                        encoding:NSUTF8StringEncoding];
                newElement_2.f = [NSNumber numberWithUnsignedChar:entry_2.f.Raw()];
                newElement_2.g = [NSNumber numberWithFloat:entry_2.g];
                newElement_2.h = [NSNumber numberWithDouble:entry_2.h];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                OnFailureFn(context, err);
                return;
            }
            newElement_0.d = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.e.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                NSNumber * newElement_2;
                newElement_2 = [NSNumber numberWithUnsignedInt:entry_2];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                OnFailureFn(context, err);
                return;
            }
            newElement_0.e = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.f.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                NSData * newElement_2;
                newElement_2 = [NSData dataWithBytes:entry_2.data() length:entry_2.size()];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                OnFailureFn(context, err);
                return;
            }
            newElement_0.f = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.g.begin();
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
            newElement_0.g = array_2;
        }
        [array_0 addObject:newElement_0];
    }
    CHIP_ERROR err = iter_0.GetStatus();
    if (err != CHIP_NO_ERROR) {
        OnFailureFn(context, err);
        return;
    }
    response.arg1 = array_0;
}
}
{ { // Scope for our temporary variables
    auto * array_0 = [NSMutableArray new];
auto iter_0 = data.arg2.begin();
while (iter_0.Next()) {
    auto & entry_0 = iter_0.GetValue();
    MTRUnitTestingClusterSimpleStruct * newElement_0;
    newElement_0 = [MTRUnitTestingClusterSimpleStruct new];
    newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
    newElement_0.b = [NSNumber numberWithBool:entry_0.b];
    newElement_0.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.c)];
    newElement_0.d = [NSData dataWithBytes:entry_0.d.data() length:entry_0.d.size()];
    newElement_0.e = [[NSString alloc] initWithBytes:entry_0.e.data() length:entry_0.e.size() encoding:NSUTF8StringEncoding];
    newElement_0.f = [NSNumber numberWithUnsignedChar:entry_0.f.Raw()];
    newElement_0.g = [NSNumber numberWithFloat:entry_0.g];
    newElement_0.h = [NSNumber numberWithDouble:entry_0.h];
    [array_0 addObject:newElement_0];
}
CHIP_ERROR err = iter_0.GetStatus();
if (err != CHIP_NO_ERROR) {
    OnFailureFn(context, err);
    return;
}
response.arg2 = array_0;
}
}
{ { // Scope for our temporary variables
    auto * array_0 = [NSMutableArray new];
auto iter_0 = data.arg3.begin();
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
response.arg3 = array_0;
}
}
{ { // Scope for our temporary variables
    auto * array_0 = [NSMutableArray new];
auto iter_0 = data.arg4.begin();
while (iter_0.Next()) {
    auto & entry_0 = iter_0.GetValue();
    NSNumber * newElement_0;
    newElement_0 = [NSNumber numberWithBool:entry_0];
    [array_0 addObject:newElement_0];
}
CHIP_ERROR err = iter_0.GetStatus();
if (err != CHIP_NO_ERROR) {
    OnFailureFn(context, err);
    return;
}
response.arg4 = array_0;
}
}
{
    response.arg5 = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.arg5)];
}
{
    response.arg6 = [NSNumber numberWithBool:data.arg6];
}
DispatchSuccess(context, response);
}
;

void MTRUnitTestingClusterTestListInt8UReverseResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestListInt8UReverseResponseParams new];
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = data.arg1.begin();
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
            response.arg1 = array_0;
        }
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEnumsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEnumsResponseParams new];
    {
        response.arg1 = [NSNumber numberWithUnsignedShort:chip::to_underlying(data.arg1)];
    }
    {
        response.arg2 = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.arg2)];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestNullableOptionalResponseParams new];
    {
        response.wasPresent = [NSNumber numberWithBool:data.wasPresent];
    }
    {
        if (data.wasNull.HasValue()) {
            response.wasNull = [NSNumber numberWithBool:data.wasNull.Value()];
        } else {
            response.wasNull = nil;
        }
    }
    {
        if (data.value.HasValue()) {
            response.value = [NSNumber numberWithUnsignedChar:data.value.Value()];
        } else {
            response.value = nil;
        }
    }
    {
        if (data.originalValue.HasValue()) {
            if (data.originalValue.Value().IsNull()) {
                response.originalValue = nil;
            } else {
                response.originalValue = [NSNumber numberWithUnsignedChar:data.originalValue.Value().Value()];
            }
        } else {
            response.originalValue = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestComplexNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestComplexNullableOptionalResponseParams new];
    {
        response.nullableIntWasNull = [NSNumber numberWithBool:data.nullableIntWasNull];
    }
    {
        if (data.nullableIntValue.HasValue()) {
            response.nullableIntValue = [NSNumber numberWithUnsignedShort:data.nullableIntValue.Value()];
        } else {
            response.nullableIntValue = nil;
        }
    }
    {
        response.optionalIntWasPresent = [NSNumber numberWithBool:data.optionalIntWasPresent];
    }
    {
        if (data.optionalIntValue.HasValue()) {
            response.optionalIntValue = [NSNumber numberWithUnsignedShort:data.optionalIntValue.Value()];
        } else {
            response.optionalIntValue = nil;
        }
    }
    {
        response.nullableOptionalIntWasPresent = [NSNumber numberWithBool:data.nullableOptionalIntWasPresent];
    }
    {
        if (data.nullableOptionalIntWasNull.HasValue()) {
            response.nullableOptionalIntWasNull = [NSNumber numberWithBool:data.nullableOptionalIntWasNull.Value()];
        } else {
            response.nullableOptionalIntWasNull = nil;
        }
    }
    {
        if (data.nullableOptionalIntValue.HasValue()) {
            response.nullableOptionalIntValue = [NSNumber numberWithUnsignedShort:data.nullableOptionalIntValue.Value()];
        } else {
            response.nullableOptionalIntValue = nil;
        }
    }
    {
        response.nullableStringWasNull = [NSNumber numberWithBool:data.nullableStringWasNull];
    }
    {
        if (data.nullableStringValue.HasValue()) {
            response.nullableStringValue = [[NSString alloc] initWithBytes:data.nullableStringValue.Value().data()
                                                                    length:data.nullableStringValue.Value().size()
                                                                  encoding:NSUTF8StringEncoding];
        } else {
            response.nullableStringValue = nil;
        }
    }
    {
        response.optionalStringWasPresent = [NSNumber numberWithBool:data.optionalStringWasPresent];
    }
    {
        if (data.optionalStringValue.HasValue()) {
            response.optionalStringValue = [[NSString alloc] initWithBytes:data.optionalStringValue.Value().data()
                                                                    length:data.optionalStringValue.Value().size()
                                                                  encoding:NSUTF8StringEncoding];
        } else {
            response.optionalStringValue = nil;
        }
    }
    {
        response.nullableOptionalStringWasPresent = [NSNumber numberWithBool:data.nullableOptionalStringWasPresent];
    }
    {
        if (data.nullableOptionalStringWasNull.HasValue()) {
            response.nullableOptionalStringWasNull = [NSNumber numberWithBool:data.nullableOptionalStringWasNull.Value()];
        } else {
            response.nullableOptionalStringWasNull = nil;
        }
    }
    {
        if (data.nullableOptionalStringValue.HasValue()) {
            response.nullableOptionalStringValue = [[NSString alloc] initWithBytes:data.nullableOptionalStringValue.Value().data()
                                                                            length:data.nullableOptionalStringValue.Value().size()
                                                                          encoding:NSUTF8StringEncoding];
        } else {
            response.nullableOptionalStringValue = nil;
        }
    }
    {
        response.nullableStructWasNull = [NSNumber numberWithBool:data.nullableStructWasNull];
    }
    {
        if (data.nullableStructValue.HasValue()) {
            response.nullableStructValue = [MTRUnitTestingClusterSimpleStruct new];
            response.nullableStructValue.a = [NSNumber numberWithUnsignedChar:data.nullableStructValue.Value().a];
            response.nullableStructValue.b = [NSNumber numberWithBool:data.nullableStructValue.Value().b];
            response.nullableStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(data.nullableStructValue.Value().c)];
            response.nullableStructValue.d = [NSData dataWithBytes:data.nullableStructValue.Value().d.data()
                                                            length:data.nullableStructValue.Value().d.size()];
            response.nullableStructValue.e = [[NSString alloc] initWithBytes:data.nullableStructValue.Value().e.data()
                                                                      length:data.nullableStructValue.Value().e.size()
                                                                    encoding:NSUTF8StringEncoding];
            response.nullableStructValue.f = [NSNumber numberWithUnsignedChar:data.nullableStructValue.Value().f.Raw()];
            response.nullableStructValue.g = [NSNumber numberWithFloat:data.nullableStructValue.Value().g];
            response.nullableStructValue.h = [NSNumber numberWithDouble:data.nullableStructValue.Value().h];
        } else {
            response.nullableStructValue = nil;
        }
    }
    {
        response.optionalStructWasPresent = [NSNumber numberWithBool:data.optionalStructWasPresent];
    }
    {
        if (data.optionalStructValue.HasValue()) {
            response.optionalStructValue = [MTRUnitTestingClusterSimpleStruct new];
            response.optionalStructValue.a = [NSNumber numberWithUnsignedChar:data.optionalStructValue.Value().a];
            response.optionalStructValue.b = [NSNumber numberWithBool:data.optionalStructValue.Value().b];
            response.optionalStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(data.optionalStructValue.Value().c)];
            response.optionalStructValue.d = [NSData dataWithBytes:data.optionalStructValue.Value().d.data()
                                                            length:data.optionalStructValue.Value().d.size()];
            response.optionalStructValue.e = [[NSString alloc] initWithBytes:data.optionalStructValue.Value().e.data()
                                                                      length:data.optionalStructValue.Value().e.size()
                                                                    encoding:NSUTF8StringEncoding];
            response.optionalStructValue.f = [NSNumber numberWithUnsignedChar:data.optionalStructValue.Value().f.Raw()];
            response.optionalStructValue.g = [NSNumber numberWithFloat:data.optionalStructValue.Value().g];
            response.optionalStructValue.h = [NSNumber numberWithDouble:data.optionalStructValue.Value().h];
        } else {
            response.optionalStructValue = nil;
        }
    }
    {
        response.nullableOptionalStructWasPresent = [NSNumber numberWithBool:data.nullableOptionalStructWasPresent];
    }
    {
        if (data.nullableOptionalStructWasNull.HasValue()) {
            response.nullableOptionalStructWasNull = [NSNumber numberWithBool:data.nullableOptionalStructWasNull.Value()];
        } else {
            response.nullableOptionalStructWasNull = nil;
        }
    }
    {
        if (data.nullableOptionalStructValue.HasValue()) {
            response.nullableOptionalStructValue = [MTRUnitTestingClusterSimpleStruct new];
            response.nullableOptionalStructValue.a = [NSNumber numberWithUnsignedChar:data.nullableOptionalStructValue.Value().a];
            response.nullableOptionalStructValue.b = [NSNumber numberWithBool:data.nullableOptionalStructValue.Value().b];
            response.nullableOptionalStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(data.nullableOptionalStructValue.Value().c)];
            response.nullableOptionalStructValue.d = [NSData dataWithBytes:data.nullableOptionalStructValue.Value().d.data()
                                                                    length:data.nullableOptionalStructValue.Value().d.size()];
            response.nullableOptionalStructValue.e =
                [[NSString alloc] initWithBytes:data.nullableOptionalStructValue.Value().e.data()
                                         length:data.nullableOptionalStructValue.Value().e.size()
                                       encoding:NSUTF8StringEncoding];
            response.nullableOptionalStructValue.f =
                [NSNumber numberWithUnsignedChar:data.nullableOptionalStructValue.Value().f.Raw()];
            response.nullableOptionalStructValue.g = [NSNumber numberWithFloat:data.nullableOptionalStructValue.Value().g];
            response.nullableOptionalStructValue.h = [NSNumber numberWithDouble:data.nullableOptionalStructValue.Value().h];
        } else {
            response.nullableOptionalStructValue = nil;
        }
    }
    {
        response.nullableListWasNull = [NSNumber numberWithBool:data.nullableListWasNull];
    }
    {
        if (data.nullableListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.nullableListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.nullableListValue = array_1;
            }
        } else {
            response.nullableListValue = nil;
        }
    }
    {
        response.optionalListWasPresent = [NSNumber numberWithBool:data.optionalListWasPresent];
    }
    {
        if (data.optionalListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.optionalListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.optionalListValue = array_1;
            }
        } else {
            response.optionalListValue = nil;
        }
    }
    {
        response.nullableOptionalListWasPresent = [NSNumber numberWithBool:data.nullableOptionalListWasPresent];
    }
    {
        if (data.nullableOptionalListWasNull.HasValue()) {
            response.nullableOptionalListWasNull = [NSNumber numberWithBool:data.nullableOptionalListWasNull.Value()];
        } else {
            response.nullableOptionalListWasNull = nil;
        }
    }
    {
        if (data.nullableOptionalListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = data.nullableOptionalListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    OnFailureFn(context, err);
                    return;
                }
                response.nullableOptionalListValue = array_1;
            }
        } else {
            response.nullableOptionalListValue = nil;
        }
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterBooleanResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterBooleanResponseParams new];
    {
        response.value = [NSNumber numberWithBool:data.value];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterSimpleStructResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterSimpleStructResponseParams new];
    {
        response.arg1 = [MTRUnitTestingClusterSimpleStruct new];
        response.arg1.a = [NSNumber numberWithUnsignedChar:data.arg1.a];
        response.arg1.b = [NSNumber numberWithBool:data.arg1.b];
        response.arg1.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.arg1.c)];
        response.arg1.d = [NSData dataWithBytes:data.arg1.d.data() length:data.arg1.d.size()];
        response.arg1.e = [[NSString alloc] initWithBytes:data.arg1.e.data()
                                                   length:data.arg1.e.size()
                                                 encoding:NSUTF8StringEncoding];
        response.arg1.f = [NSNumber numberWithUnsignedChar:data.arg1.f.Raw()];
        response.arg1.g = [NSNumber numberWithFloat:data.arg1.g];
        response.arg1.h = [NSNumber numberWithDouble:data.arg1.h];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEmitTestEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEmitTestEventResponseParams new];
    {
        response.value = [NSNumber numberWithUnsignedLongLong:data.value];
    }
    DispatchSuccess(context, response);
};

void MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType & data)
{
    auto * response = [MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams new];
    {
        response.value = [NSNumber numberWithUnsignedLongLong:data.value];
    }
    DispatchSuccess(context, response);
};

void MTRIdentifyClusterIdentifyEffectIdentifierAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyEffectIdentifier value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyEffectIdentifierAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::IdentifyEffectIdentifier> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::IdentifyEffectVariant> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyIdentifyType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyIdentifyTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::Identify::IdentifyIdentifyType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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

void MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::CalendarType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterCalendarTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeFormatLocalization::CalendarType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::HourFormat value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterHourFormatAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TimeFormatLocalization::HourFormat> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationClusterTempUnitAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::UnitLocalization::TempUnit value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableUnitLocalizationClusterTempUnitAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::UnitLocalization::TempUnit> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeFault> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeLevelAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeLevel value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeLevelAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeLevel> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeState value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeStateAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatChargeState> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatFault> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatReplaceabilityAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatReplaceability value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatReplaceabilityAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::BatReplaceability> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterPowerSourceStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::PowerSourceStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterPowerSourceStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::PowerSourceStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredCurrentType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredCurrentTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::WiredCurrentType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PowerSource::WiredFault> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::CommissioningError value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterCommissioningErrorAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::CommissioningError> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::
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

void MTRNetworkCommissioningClusterWiFiBandAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::WiFiBand value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterWiFiBandAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::WiFiBand> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsIntentAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsIntent value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsIntentAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::LogsIntent> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::LogsStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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

void MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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

void MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterPumpControlModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullablePumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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

void MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::IlluminanceMeasurement::LightSensorType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
    }

    if (mEstablishedHandler != nil) {
        dispatch_async(mQueue, mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        mEstablishedHandler = nil;
    }
}

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::IlluminanceMeasurement::LightSensorType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished()
{
    if (!mQueue) {
        return;
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
