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

#import "MTRCallbackBridge_internal.h"
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

void MTROctetStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROctetStringAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROctetStringAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableOctetStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableOctetStringAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOctetStringAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRCharStringAttributeCallbackBridge::OnSuccessFn(void * context, chip::CharSpan value)
{
    NSString * _Nonnull objCValue;
    objCValue = [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
    DispatchSuccess(context, objCValue);
};

void MTRCharStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRCharStringAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRCharStringAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableCharStringAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableCharStringAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableCharStringAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBooleanAttributeCallbackBridge::OnSuccessFn(void * context, bool value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithBool:value];
    DispatchSuccess(context, objCValue);
};

void MTRBooleanAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBooleanAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBooleanAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableBooleanAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableBooleanAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableBooleanAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt8uAttributeCallbackBridge::OnSuccessFn(void * context, uint8_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt8uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt8uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt8uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt8uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt8uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt8uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt8sAttributeCallbackBridge::OnSuccessFn(void * context, int8_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithChar:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt8sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt8sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt8sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt8sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt8sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt8sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt16uAttributeCallbackBridge::OnSuccessFn(void * context, uint16_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt16uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt16uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt16uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt16uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt16uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt16uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt16sAttributeCallbackBridge::OnSuccessFn(void * context, int16_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithShort:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt16sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt16sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt16sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt16sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt16sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt16sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt32uAttributeCallbackBridge::OnSuccessFn(void * context, uint32_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt32uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt32uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt32uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt32uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt32uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt32uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt32sAttributeCallbackBridge::OnSuccessFn(void * context, int32_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithInt:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt32sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt32sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt32sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt32sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt32sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt32sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt64uAttributeCallbackBridge::OnSuccessFn(void * context, uint64_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedLongLong:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt64uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt64uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt64uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt64uAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt64uAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt64uAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRInt64sAttributeCallbackBridge::OnSuccessFn(void * context, int64_t value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithLongLong:value];
    DispatchSuccess(context, objCValue);
};

void MTRInt64sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRInt64sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRInt64sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableInt64sAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableInt64sAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableInt64sAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRFloatAttributeCallbackBridge::OnSuccessFn(void * context, float value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithFloat:value];
    DispatchSuccess(context, objCValue);
};

void MTRFloatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFloatAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFloatAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableFloatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableFloatAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableFloatAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoubleAttributeCallbackBridge::OnSuccessFn(void * context, double value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithDouble:value];
    DispatchSuccess(context, objCValue);
};

void MTRDoubleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoubleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoubleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoubleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoubleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoubleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRVendorIdAttributeCallbackBridge::OnSuccessFn(void * context, chip::VendorId value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRVendorIdAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRVendorIdAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRVendorIdAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableVendorIdAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableVendorIdAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableVendorIdAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIdentifyGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIdentifyAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIdentifyAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRScenesGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRScenesGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRScenesGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRScenesAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRScenesAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRScenesAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRScenesAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRScenesAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRScenesAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffSwitchConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROnOffSwitchConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffSwitchConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROnOffSwitchConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROnOffSwitchConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffSwitchConfigurationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffSwitchConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLevelControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLevelControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLevelControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLevelControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLevelControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLevelControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLevelControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLevelControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLevelControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBinaryInputBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBinaryInputBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBinaryInputBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBinaryInputBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBinaryInputBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBinaryInputBasicAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBinaryInputBasicAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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
            newElement_0.type = [NSNumber numberWithUnsignedInt:entry_0.type];
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

void MTRDescriptorDeviceTypeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorDeviceTypeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorDeviceTypeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorServerListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorServerListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorServerListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorClientListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorClientListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorClientListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorPartsListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorPartsListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorPartsListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDescriptorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDescriptorAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDescriptorAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBindingBindingListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBindingBindingListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBindingBindingListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBindingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBindingGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBindingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBindingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBindingAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBindingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBindingAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBindingAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBindingAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAccessControlAclListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::AccessControl::Structs::AccessControlEntry::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAccessControlClusterAccessControlEntry * newElement_0;
            newElement_0 = [MTRAccessControlClusterAccessControlEntry new];
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

void MTRAccessControlAclListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlAclListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlAclListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAccessControlExtensionListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::AccessControl::Structs::ExtensionEntry::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRAccessControlClusterExtensionEntry * newElement_0;
            newElement_0 = [MTRAccessControlClusterExtensionEntry new];
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

void MTRAccessControlExtensionListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlExtensionListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlExtensionListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccessControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccessControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccessControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRActionsActionListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsActionListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsActionListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRActionsEndpointListsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsEndpointListsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsEndpointListsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRActionsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRActionsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRActionsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBasicCapabilityMinimaStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Basic::Structs::CapabilityMinimaStruct::DecodableType & value)
{
    MTRBasicClusterCapabilityMinimaStruct * _Nonnull objCValue;
    objCValue = [MTRBasicClusterCapabilityMinimaStruct new];
    objCValue.caseSessionsPerFabric = [NSNumber numberWithUnsignedShort:value.caseSessionsPerFabric];
    objCValue.subscriptionsPerFabric = [NSNumber numberWithUnsignedShort:value.subscriptionsPerFabric];
    DispatchSuccess(context, objCValue);
};

void MTRBasicCapabilityMinimaStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBasicCapabilityMinimaStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBasicCapabilityMinimaStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBasicGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBasicAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBasicAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTRBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBasicAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBasicAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateProviderGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateProviderGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateProviderAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateProviderAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTROtaSoftwareUpdateProviderAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateProviderAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorDefaultOtaProvidersListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROtaSoftwareUpdateRequestorClusterProviderLocation * newElement_0;
            newElement_0 = [MTROtaSoftwareUpdateRequestorClusterProviderLocation new];
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

void MTROtaSoftwareUpdateRequestorDefaultOtaProvidersListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateRequestorDefaultOtaProvidersListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorDefaultOtaProvidersListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTROtaSoftwareUpdateRequestorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateRequestorAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLocalizationConfigurationSupportedLocalesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLocalizationConfigurationSupportedLocalesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationSupportedLocalesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLocalizationConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRLocalizationConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLocalizationConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRLocalizationConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLocalizationConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLocalizationConfigurationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLocalizationConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationSupportedCalendarTypesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTimeFormatLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTimeFormatLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTimeFormatLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUnitLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUnitLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUnitLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUnitLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUnitLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUnitLocalizationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceConfigurationSourcesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceConfigurationSourcesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationSourcesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPowerSourceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPowerSourceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceConfigurationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceActiveWiredFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceActiveWiredFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveWiredFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceActiveBatFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceActiveBatFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveBatFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceActiveBatChargeFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceActiveBatChargeFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceActiveBatChargeFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPowerSourceAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralCommissioningBasicCommissioningInfoStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningBasicCommissioningInfoStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningBasicCommissioningInfoStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNetworkCommissioningNetworksListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNetworkCommissioningNetworksListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningNetworksListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNetworkCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNetworkCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNetworkCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNetworkCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNetworkCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNetworkCommissioningAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDiagnosticLogsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDiagnosticLogsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDiagnosticLogsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::GeneralDiagnostics::Structs::NetworkInterfaceType::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRGeneralDiagnosticsClusterNetworkInterfaceType * newElement_0;
            newElement_0 = [MTRGeneralDiagnosticsClusterNetworkInterfaceType new];
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

void MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsNetworkInterfacesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackBridge::OnSuccessFn(
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

void MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveHardwareFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackBridge::OnSuccessFn(
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

void MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveRadioFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackBridge::OnSuccessFn(
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

void MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsActiveNetworkFaultsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGeneralDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::SoftwareDiagnostics::Structs::ThreadMetrics::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRSoftwareDiagnosticsClusterThreadMetrics * newElement_0;
            newElement_0 = [MTRSoftwareDiagnosticsClusterThreadMetrics new];
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

void MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsThreadMetricsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSoftwareDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSoftwareDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSoftwareDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSoftwareDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSoftwareDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSoftwareDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSoftwareDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThreadNetworkDiagnosticsNeighborTableListListAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRThreadNetworkDiagnosticsNeighborTableListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsNeighborTableListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsNeighborTableListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThreadNetworkDiagnosticsRouteTableListListAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRThreadNetworkDiagnosticsRouteTableListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsRouteTableListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsRouteTableListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsSecurityPolicyStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsSecurityPolicyStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsSecurityPolicyStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsOperationalDatasetComponentsStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRThreadNetworkDiagnosticsOperationalDatasetComponentsStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsOperationalDatasetComponentsStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsActiveNetworkFaultsListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThreadNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWiFiNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWiFiNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWiFiNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTREthernetNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTREthernetNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTREthernetNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTREthernetNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTREthernetNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTREthernetNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBridgedDeviceBasicGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBridgedDeviceBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBridgedDeviceBasicAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRBridgedDeviceBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBridgedDeviceBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRBridgedDeviceBasicAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTRBridgedDeviceBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBridgedDeviceBasicAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBridgedDeviceBasicAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSwitchGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSwitchGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSwitchGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSwitchAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSwitchAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSwitchAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRSwitchAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRSwitchAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRSwitchAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAdministratorCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRAdministratorCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAdministratorCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRAdministratorCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAdministratorCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAdministratorCommissioningAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROperationalCredentialsNOCsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROperationalCredentialsNOCsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsNOCsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROperationalCredentialsFabricsListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::OperationalCredentials::Structs::FabricDescriptor::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTROperationalCredentialsClusterFabricDescriptor * newElement_0;
            newElement_0 = [MTROperationalCredentialsClusterFabricDescriptor new];
            newElement_0.rootPublicKey = [NSData dataWithBytes:entry_0.rootPublicKey.data() length:entry_0.rootPublicKey.size()];
            newElement_0.vendorId = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.vendorId)];
            newElement_0.fabricId = [NSNumber numberWithUnsignedLongLong:entry_0.fabricId];
            newElement_0.nodeId = [NSNumber numberWithUnsignedLongLong:entry_0.nodeId];
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

void MTROperationalCredentialsFabricsListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROperationalCredentialsFabricsListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsFabricsListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROperationalCredentialsTrustedRootCertificatesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROperationalCredentialsTrustedRootCertificatesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsTrustedRootCertificatesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROperationalCredentialsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROperationalCredentialsGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROperationalCredentialsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROperationalCredentialsAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROperationalCredentialsAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROperationalCredentialsAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupKeyManagementGroupKeyMapListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementGroupKeyMapListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGroupKeyMapListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupKeyManagementGroupTableListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementGroupTableListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGroupTableListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupKeyManagementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupKeyManagementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRGroupKeyManagementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFixedLabelLabelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFixedLabelLabelListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFixedLabelLabelListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFixedLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFixedLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFixedLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFixedLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFixedLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFixedLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFixedLabelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFixedLabelAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFixedLabelAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUserLabelLabelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUserLabelLabelListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUserLabelLabelListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUserLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUserLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUserLabelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUserLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUserLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUserLabelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRUserLabelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUserLabelAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUserLabelAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBooleanStateGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBooleanStateGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBooleanStateGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBooleanStateAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBooleanStateAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBooleanStateAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBooleanStateAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBooleanStateAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBooleanStateAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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
                    MTRModeSelectClusterSemanticTag * newElement_2;
                    newElement_2 = [MTRModeSelectClusterSemanticTag new];
                    newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:entry_2.mfgCode];
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

void MTRModeSelectSupportedModesListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRModeSelectSupportedModesListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRModeSelectSupportedModesListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRModeSelectGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRModeSelectGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRModeSelectGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRModeSelectAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRModeSelectAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRModeSelectAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRModeSelectAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRModeSelectAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRModeSelectAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockCredentialRulesSupportAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockCredentialRulesSupportAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockCredentialRulesSupportAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockCredentialRulesSupportAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockSupportedOperatingModesAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockSupportedOperatingModesAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockSupportedOperatingModesAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockSupportedOperatingModesAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockDefaultConfigurationRegisterAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockDefaultConfigurationRegisterAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockDefaultConfigurationRegisterAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockDefaultConfigurationRegisterAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockLocalProgrammingFeaturesAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockLocalProgrammingFeaturesAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockLocalProgrammingFeaturesAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockLocalProgrammingFeaturesAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDoorLockGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDoorLockAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRDoorLockAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringConfigStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringConfigStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringConfigStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringConfigStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringOperationalStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringOperationalStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringOperationalStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringOperationalStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::Mode> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringSafetyStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringSafetyStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringSafetyStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringSafetyStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWindowCoveringGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWindowCoveringAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRWindowCoveringAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBarrierControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBarrierControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBarrierControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBarrierControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBarrierControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBarrierControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBarrierControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBarrierControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBarrierControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPumpConfigurationAndControlPumpStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlPumpStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlPumpStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlPumpStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPumpConfigurationAndControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPumpConfigurationAndControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPumpConfigurationAndControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFanControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFanControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFanControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFanControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFanControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFanControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFanControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFanControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFanControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatUserInterfaceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRThermostatUserInterfaceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(
        context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatUserInterfaceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRThermostatUserInterfaceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRThermostatUserInterfaceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRThermostatUserInterfaceConfigurationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatUserInterfaceConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRColorControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRColorControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRColorControlAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBallastConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBallastConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBallastConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBallastConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRBallastConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRBallastConfigurationAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRBallastConfigurationAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIlluminanceMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIlluminanceMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIlluminanceMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIlluminanceMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRIlluminanceMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIlluminanceMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTemperatureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTemperatureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTemperatureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTemperatureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTemperatureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTemperatureMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTemperatureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPressureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPressureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPressureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPressureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRPressureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPressureMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPressureMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFlowMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFlowMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFlowMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFlowMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRFlowMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFlowMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFlowMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRRelativeHumidityMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRRelativeHumidityMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRRelativeHumidityMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRRelativeHumidityMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRRelativeHumidityMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRRelativeHumidityMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRRelativeHumidityMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROccupancySensingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROccupancySensingGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROccupancySensingGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROccupancySensingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROccupancySensingAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROccupancySensingAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTROccupancySensingAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROccupancySensingAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROccupancySensingAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWakeOnLanGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLanGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWakeOnLanGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWakeOnLanGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWakeOnLanAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRWakeOnLanAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWakeOnLanAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWakeOnLanAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWakeOnLanAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTRWakeOnLanAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWakeOnLanAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWakeOnLanAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelChannelListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelChannelListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelChannelListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelLineupStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelLineupStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelLineupStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelCurrentChannelStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelCurrentChannelStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelCurrentChannelStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRChannelAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTargetNavigatorTargetListListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::TargetNavigator::Structs::TargetInfo::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRTargetNavigatorClusterTargetInfo * newElement_0;
            newElement_0 = [MTRTargetNavigatorClusterTargetInfo new];
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

void MTRTargetNavigatorTargetListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTargetNavigatorTargetListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorTargetListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTargetNavigatorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTargetNavigatorGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTargetNavigatorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTargetNavigatorAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRTargetNavigatorAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTargetNavigatorAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaPlaybackSampledPositionStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackSampledPositionStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackSampledPositionStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaPlaybackGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaPlaybackAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaPlaybackAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaInputInputListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaInputInputListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaInputInputListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaInputGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaInputAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRMediaInputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaInputAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaInputAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLowPowerGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLowPowerGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLowPowerGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLowPowerAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLowPowerAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLowPowerAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRLowPowerAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLowPowerAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLowPowerAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRKeypadInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRKeypadInputGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRKeypadInputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRKeypadInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRKeypadInputAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRKeypadInputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRKeypadInputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRKeypadInputAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRKeypadInputAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRContentLauncherAcceptHeaderListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherAcceptHeaderListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAcceptHeaderListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRContentLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRContentLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRContentLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAudioOutputOutputListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAudioOutputOutputListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAudioOutputOutputListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAudioOutputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAudioOutputGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAudioOutputGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAudioOutputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAudioOutputAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAudioOutputAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAudioOutputAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAudioOutputAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAudioOutputAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationLauncherCatalogListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationLauncherCatalogListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherCatalogListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationLauncherCurrentAppStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationLauncherCurrentAppStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherCurrentAppStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationLauncherAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationBasicApplicationStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicApplicationStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicApplicationStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationBasicAllowedVendorListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicAllowedVendorListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAllowedVendorListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRApplicationBasicAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccountLoginGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccountLoginGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccountLoginGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccountLoginAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccountLoginAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccountLoginAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRAccountLoginAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccountLoginAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccountLoginAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRElectricalMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRElectricalMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRElectricalMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRElectricalMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRElectricalMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRElectricalMeasurementAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRElectricalMeasurementAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterBitmap8AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterBitmap8AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterBitmap8AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterBitmap8AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterBitmap16AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedShort:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterBitmap16AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterBitmap16AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterBitmap16AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterBitmap32AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedInt:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterBitmap32AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterBitmap32AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterBitmap32AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterBitmap64AttributeCallbackBridge::OnSuccessFn(
    void * context, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap> value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedLongLong:value.Raw()];
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterBitmap64AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterBitmap64AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterBitmap64AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListInt8uListAttributeCallbackBridge::OnSuccessFn(
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

void MTRTestClusterListInt8uListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListInt8uListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListInt8uListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListOctetStringListAttributeCallbackBridge::OnSuccessFn(
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

void MTRTestClusterListOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListOctetStringListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListOctetStringListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListStructOctetStringListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::TestCluster::Structs::TestListStructOctet::DecodableType> &
        value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRTestClusterClusterTestListStructOctet * newElement_0;
            newElement_0 = [MTRTestClusterClusterTestListStructOctet new];
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

void MTRTestClusterListStructOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListStructOctetStringListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListStructOctetStringListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListNullablesAndOptionalsStructListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<
        chip::app::Clusters::TestCluster::Structs::NullablesAndOptionalsStruct::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRTestClusterClusterNullablesAndOptionalsStruct * newElement_0;
            newElement_0 = [MTRTestClusterClusterNullablesAndOptionalsStruct new];
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
                newElement_0.nullableStruct = [MTRTestClusterClusterSimpleStruct new];
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
                newElement_0.optionalStruct = [MTRTestClusterClusterSimpleStruct new];
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
                    newElement_0.nullableOptionalStruct = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterListNullablesAndOptionalsStructListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListNullablesAndOptionalsStructListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListNullablesAndOptionalsStructListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterStructAttrStructAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Structs::SimpleStruct::DecodableType & value)
{
    MTRTestClusterClusterSimpleStruct * _Nonnull objCValue;
    objCValue = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterStructAttrStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterStructAttrStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterStructAttrStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListLongOctetStringListAttributeCallbackBridge::OnSuccessFn(
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

void MTRTestClusterListLongOctetStringListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListLongOctetStringListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListLongOctetStringListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterListFabricScopedListAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::TestCluster::Structs::TestFabricScoped::DecodableType> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRTestClusterClusterTestFabricScoped * newElement_0;
            newElement_0 = [MTRTestClusterClusterTestFabricScoped new];
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
            newElement_0.fabricSensitiveStruct = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterListFabricScopedListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterListFabricScopedListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterListFabricScopedListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterNullableBitmap8AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterNullableBitmap8AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterNullableBitmap8AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterNullableBitmap8AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterNullableBitmap16AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedShort:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterNullableBitmap16AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterNullableBitmap16AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterNullableBitmap16AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterNullableBitmap32AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedInt:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterNullableBitmap32AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterNullableBitmap32AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterNullableBitmap32AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterNullableBitmap64AttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap>> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedLongLong:value.Value().Raw()];
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterNullableBitmap64AttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterNullableBitmap64AttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterNullableBitmap64AttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterNullableStructStructAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::TestCluster::Structs::SimpleStruct::DecodableType> & value)
{
    MTRTestClusterClusterSimpleStruct * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterNullableStructStructAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterNullableStructStructAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterNullableStructStructAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterGeneratedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterGeneratedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterGeneratedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterAcceptedCommandListListAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::DecodableList<chip::CommandId> & value)
{
    NSArray * _Nonnull objCValue;
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = value.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            OnFailureFn(context, err);
            return;
        }
        objCValue = array_0;
    }
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterAcceptedCommandListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterAcceptedCommandListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterAttributeListListAttributeCallbackBridge::OnSuccessFn(
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

void MTRTestClusterAttributeListListAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterAttributeListListAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterAttributeListListAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGroupsClusterAddGroupResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType & data)
{
    auto * response = [MTRGroupsClusterAddGroupResponseParams new];
    {
        response.status = [NSNumber numberWithUnsignedChar:data.status];
    }
    {
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
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
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
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
        response.groupId = [NSNumber numberWithUnsignedShort:data.groupId];
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

void MTROtaSoftwareUpdateProviderClusterQueryImageResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType & data)
{
    auto * response = [MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams new];
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

void MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType & data)
{
    auto * response = [MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams new];
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
        response.content = [NSData dataWithBytes:data.content.data() length:data.content.size()];
    }
    {
        response.timeStamp = [NSNumber numberWithUnsignedInt:data.timeStamp];
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
        response.signature = [NSData dataWithBytes:data.signature.data() length:data.signature.size()];
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
        if (data.userUniqueId.IsNull()) {
            response.userUniqueId = nil;
        } else {
            response.userUniqueId = [NSNumber numberWithUnsignedInt:data.userUniqueId.Value()];
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
                    MTRDoorLockClusterDlCredential * newElement_1;
                    newElement_1 = [MTRDoorLockClusterDlCredential new];
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

void MTRTestClusterClusterTestSpecificResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestSpecificResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestSpecificResponseParams new];
    {
        response.returnValue = [NSNumber numberWithUnsignedChar:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterTestAddArgumentsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestAddArgumentsResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestAddArgumentsResponseParams new];
    {
        response.returnValue = [NSNumber numberWithUnsignedChar:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterTestSimpleArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestSimpleArgumentResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestSimpleArgumentResponseParams new];
    {
        response.returnValue = [NSNumber numberWithBool:data.returnValue];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterTestStructArrayArgumentResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestStructArrayArgumentResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestStructArrayArgumentResponseParams new];
    { { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
    auto iter_0 = data.arg1.begin();
    while (iter_0.Next()) {
        auto & entry_0 = iter_0.GetValue();
        MTRTestClusterClusterNestedStructList * newElement_0;
        newElement_0 = [MTRTestClusterClusterNestedStructList new];
        newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
        newElement_0.b = [NSNumber numberWithBool:entry_0.b];
        newElement_0.c = [MTRTestClusterClusterSimpleStruct new];
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
                MTRTestClusterClusterSimpleStruct * newElement_2;
                newElement_2 = [MTRTestClusterClusterSimpleStruct new];
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
    MTRTestClusterClusterSimpleStruct * newElement_0;
    newElement_0 = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterClusterTestListInt8UReverseResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestListInt8UReverseResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestListInt8UReverseResponseParams new];
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

void MTRTestClusterClusterTestEnumsResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestEnumsResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestEnumsResponseParams new];
    {
        response.arg1 = [NSNumber numberWithUnsignedShort:chip::to_underlying(data.arg1)];
    }
    {
        response.arg2 = [NSNumber numberWithUnsignedChar:chip::to_underlying(data.arg2)];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterTestNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestNullableOptionalResponseParams new];
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

void MTRTestClusterClusterTestComplexNullableOptionalResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestComplexNullableOptionalResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestComplexNullableOptionalResponseParams new];
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
            response.nullableStructValue = [MTRTestClusterClusterSimpleStruct new];
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
            response.optionalStructValue = [MTRTestClusterClusterSimpleStruct new];
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
            response.nullableOptionalStructValue = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterClusterBooleanResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::BooleanResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterBooleanResponseParams new];
    {
        response.value = [NSNumber numberWithBool:data.value];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterSimpleStructResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::SimpleStructResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterSimpleStructResponseParams new];
    {
        response.arg1 = [MTRTestClusterClusterSimpleStruct new];
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

void MTRTestClusterClusterTestEmitTestEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestEmitTestEventResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestEmitTestEventResponseParams new];
    {
        response.value = [NSNumber numberWithUnsignedLongLong:data.value];
    }
    DispatchSuccess(context, response);
};

void MTRTestClusterClusterTestEmitTestFabricScopedEventResponseCallbackBridge::OnSuccessFn(
    void * context, const chip::app::Clusters::TestCluster::Commands::TestEmitTestFabricScopedEventResponse::DecodableType & data)
{
    auto * response = [MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams new];
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

void MTRIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyEffectIdentifierAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Identify::IdentifyIdentifyType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableIdentifyClusterIdentifyIdentifyTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffDelayedAllOffEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableOnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableOnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffDelayedAllOffEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffDyingLightEffectVariant value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableOnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableOnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffDyingLightEffectVariantAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROnOffClusterOnOffEffectIdentifierAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffEffectIdentifier value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableOnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableOnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffEffectIdentifierAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROnOffClusterOnOffStartUpOnOffAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OnOff::OnOffStartUpOnOff value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableOnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableOnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOnOffClusterOnOffStartUpOnOffAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRLevelControlClusterMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::LevelControl::MoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableLevelControlClusterMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRLevelControlClusterStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::LevelControl::StepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRLevelControlClusterStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableLevelControlClusterStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableLevelControlClusterStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAccessControlClusterAuthModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::AuthMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableAccessControlClusterAuthModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AccessControl::AuthMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterAuthModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAccessControlClusterChangeTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::ChangeTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterChangeTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAccessControlClusterPrivilegeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AccessControl::Privilege value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableAccessControlClusterPrivilegeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::AccessControl::Privilege> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAccessControlClusterPrivilegeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRActionsClusterActionErrorEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionErrorEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionErrorEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRActionsClusterActionStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRActionsClusterActionTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::ActionTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterActionTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRActionsClusterEndpointListTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Actions::EndpointListTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableActionsClusterEndpointListTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTROtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRNullableOtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableOtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge *>(
        context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTAApplyUpdateActionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRNullableOtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableOtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTADownloadProtocolAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackBridge::OnSuccessFn(
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

void MTRNullableOtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableOtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateProviderClusterOTAQueryStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTROtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished(void * context)
{
    auto * self
        = static_cast<MTRNullableOtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge *>(
            context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAAnnouncementReasonAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTROtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableOtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge *>(
        context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAChangeReasonEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackBridge::OnSuccessFn(void * context,
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

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableOtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOtaSoftwareUpdateRequestorClusterOTAUpdateStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::CalendarType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterCalendarTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeFormatLocalization::HourFormat value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTimeFormatLocalizationClusterHourFormatAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRUnitLocalizationClusterTempUnitAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::UnitLocalization::TempUnit value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableUnitLocalizationClusterTempUnitAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterBatChargeFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterBatChargeLevelAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeLevel value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeLevelAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterBatChargeStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatChargeState value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatChargeStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterBatFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterBatReplaceabilityAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::BatReplaceability value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterBatReplaceabilityAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterPowerSourceStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::PowerSourceStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterPowerSourceStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredCurrentType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredCurrentTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPowerSourceClusterWiredFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PowerSource::WiredFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRPowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullablePowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePowerSourceClusterWiredFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::CommissioningError value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterCommissioningErrorAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralCommissioningClusterRegulatoryLocationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge *>(
            context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterNetworkCommissioningStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNetworkCommissioningClusterWiFiBandAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::NetworkCommissioning::WiFiBand value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableNetworkCommissioningClusterWiFiBandAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDiagnosticLogsClusterLogsIntentAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsIntent value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsIntentAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDiagnosticLogsClusterLogsStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDiagnosticLogsClusterLogsTransferProtocolAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::BootReasonType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::BootReasonType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterBootReasonTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::HardwareFaultType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::HardwareFaultType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterHardwareFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::InterfaceType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::InterfaceType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterInterfaceTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::NetworkFaultType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::NetworkFaultType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterNetworkFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GeneralDiagnostics::RadioFaultType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::GeneralDiagnostics::RadioFaultType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGeneralDiagnosticsClusterRadioFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFault value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterNetworkFaultAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterRoutingRoleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ThreadNetworkDiagnostics::ThreadConnectionStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::ThreadConnectionStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge *>(
            context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThreadNetworkDiagnosticsClusterThreadConnectionStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge *>(
        context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterAssociationFailureCauseAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterSecurityTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiConnectionStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWiFiNetworkDiagnosticsClusterWiFiVersionTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTREthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTREthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTREthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTREthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableEthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableEthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableEthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableEthernetNetworkDiagnosticsClusterPHYRateTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeSynchronization::GranularityEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTimeSynchronizationClusterGranularityEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TimeSynchronization::TimeSourceEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTimeSynchronizationClusterTimeSourceEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge::
    OnSubscriptionEstablished(void * context)
{
    auto * self
        = static_cast<MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge *>(
            context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAdministratorCommissioningClusterCommissioningWindowStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AdministratorCommissioning::StatusCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAdministratorCommissioningClusterStatusCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTROperationalCredentialsClusterOperationalCertStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::OperationalCredentials::OperationalCertStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTROperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTROperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTROperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableOperationalCredentialsClusterOperationalCertStatusAttributeCallbackBridge::OnSuccessFn(void * context,
    const chip::app::DataModel::Nullable<chip::app::Clusters::OperationalCredentials::OperationalCertStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableOperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableOperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableOperationalCredentialsClusterOperationalCertStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableGroupKeyManagementClusterGroupKeySecurityPolicyAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlAlarmCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlAlarmCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlAlarmCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlAlarmCode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlAlarmCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlCredentialRuleAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlCredentialRule value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlCredentialRuleAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlCredentialRule> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlCredentialRuleAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlCredentialTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlCredentialType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlCredentialTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlCredentialType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlCredentialTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlDataOperationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlDataOperationType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlDataOperationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDataOperationType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlDataOperationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlDoorStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlDoorState value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlDoorStateAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlDoorStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlLockDataTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockDataType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlLockDataTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockDataType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockDataTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlLockOperationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockOperationType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlLockOperationTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockOperationType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockOperationTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlLockStateAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockState value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockStateAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlLockTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlLockType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlLockTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlOperatingModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlOperatingMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlOperatingModeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlOperatingMode> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlOperatingModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlOperationErrorAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlOperationError value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlOperationErrorAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlOperationError> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlOperationErrorAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlOperationSourceAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlOperationSource value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlOperationSourceAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlOperationSource> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlOperationSourceAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlStatusAttributeCallbackBridge::OnSuccessFn(void * context, chip::app::Clusters::DoorLock::DlStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlUserStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlUserStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlUserStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlUserStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlUserStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDlUserTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DlUserType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDlUserTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlUserType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDlUserTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockOperationEventCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockOperationEventCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockProgrammingEventCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockProgrammingEventCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockSetPinOrIdStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockSetPinOrIdStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDoorLockUserStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockUserStatus value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDoorLockUserStatusAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorLockUserStatus> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockUserStatusAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRDoorLockClusterDoorLockUserTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::DoorLock::DoorLockUserType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableDoorLockClusterDoorLockUserTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DoorLockUserType> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableDoorLockClusterDoorLockUserTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringClusterEndProductTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WindowCovering::EndProductType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWindowCoveringClusterEndProductTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRWindowCoveringClusterTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::WindowCovering::Type value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableWindowCoveringClusterTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullablePumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterPumpControlModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRPumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullablePumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullablePumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullablePumpConfigurationAndControlClusterPumpOperationModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThermostatClusterSetpointAdjustModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::SetpointAdjustMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterSetpointAdjustModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThermostatClusterThermostatControlSequenceAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatControlSequence value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatControlSequenceAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThermostatClusterThermostatRunningModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatRunningMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatRunningModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRThermostatClusterThermostatSystemModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Thermostat::ThermostatSystemMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableThermostatClusterThermostatSystemModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRFanControlClusterFanModeSequenceTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::FanModeSequenceType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterFanModeSequenceTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRFanControlClusterFanModeTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FanControl::FanModeType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableFanControlClusterFanModeTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterColorLoopActionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorLoopAction value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorLoopActionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterColorLoopDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorLoopDirection value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorLoopDirectionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterColorModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::ColorMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterColorModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterColorModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterColorModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterHueDirectionAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueDirection value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueDirectionAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterHueMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueMoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterHueStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::HueStepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterHueStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterSaturationMoveModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::SaturationMoveMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterSaturationMoveModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRColorControlClusterSaturationStepModeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ColorControl::SaturationStepMode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableColorControlClusterSaturationStepModeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::IlluminanceMeasurement::LightSensorType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableIlluminanceMeasurementClusterLightSensorTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRChannelClusterChannelStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Channel::ChannelStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableChannelClusterChannelStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRChannelClusterLineupInfoTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::Channel::LineupInfoTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableChannelClusterLineupInfoTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TargetNavigator::TargetNavigatorStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTargetNavigatorClusterTargetNavigatorStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaPlayback::MediaPlaybackStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableMediaPlaybackClusterMediaPlaybackStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaPlayback::PlaybackStateEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableMediaPlaybackClusterPlaybackStateEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRMediaInputClusterInputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::MediaInput::InputTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableMediaInputClusterInputTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRKeypadInputClusterCecKeyCodeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::KeypadInput::CecKeyCode value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableKeypadInputClusterCecKeyCodeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::KeypadInput::KeypadInputStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self = static_cast<MTRNullableKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableKeypadInputClusterKeypadInputStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterContentLaunchStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRContentLauncherClusterMetricTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::MetricTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterMetricTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRContentLauncherClusterParameterEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ContentLauncher::ParameterEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableContentLauncherClusterParameterEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRAudioOutputClusterOutputTypeEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::AudioOutput::OutputTypeEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableAudioOutputClusterOutputTypeEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ApplicationLauncher::ApplicationLauncherStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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
    OnSubscriptionEstablished(void * context)
{
    auto * self
        = static_cast<MTRNullableApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge *>(
            context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableApplicationLauncherClusterApplicationLauncherStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(
    void * context)
{
    auto * self
        = static_cast<MTRNullableApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableApplicationBasicClusterApplicationStatusEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRTestClusterClusterSimpleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::TestCluster::SimpleEnum value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRNullableTestClusterClusterSimpleEnumAttributeCallbackBridge::OnSuccessFn(
    void * context, const chip::app::DataModel::Nullable<chip::app::Clusters::TestCluster::SimpleEnum> & value)
{
    NSNumber * _Nullable objCValue;
    if (value.IsNull()) {
        objCValue = nil;
    } else {
        objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value.Value())];
    }
    DispatchSuccess(context, objCValue);
};

void MTRNullableTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableTestClusterClusterSimpleEnumAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}

void MTRFaultInjectionClusterFaultTypeAttributeCallbackBridge::OnSuccessFn(
    void * context, chip::app::Clusters::FaultInjection::FaultType value)
{
    NSNumber * _Nonnull objCValue;
    objCValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(value)];
    DispatchSuccess(context, objCValue);
};

void MTRFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
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

void MTRNullableFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnSubscriptionEstablished(void * context)
{
    auto * self = static_cast<MTRNullableFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge *>(context);
    if (!self->mQueue) {
        return;
    }

    if (self->mEstablishedHandler != nil) {
        dispatch_async(self->mQueue, self->mEstablishedHandler);
        // On failure, mEstablishedHandler will be cleaned up by our destructor,
        // but we can clean it up earlier on successful subscription
        // establishment.
        self->mEstablishedHandler = nil;
    }
}

void MTRNullableFaultInjectionClusterFaultTypeAttributeCallbackSubscriptionBridge::OnDone()
{
    if (!mQueue) {
        delete this;
        return;
    }

    // Delete ourselves async, so that any error/data reports we
    // queued up before getting OnDone have a chance to run.
    auto * self = this;
    dispatch_async(mQueue, ^{
        delete self;
    });
}
