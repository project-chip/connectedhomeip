/**
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

#import "MTRError_Utils.h"
#import "MTRLogging.h"
#import <Matter/Matter.h>

#import <app/MessageDef/StatusIB.h>
#import <app/util/af-enums.h>
#import <app/util/error-mapping.h>
#import <inet/InetError.h>
#import <lib/support/TypeTraits.h>

// Stolen for now from the framework, need to export this properly.
@interface MTRErrorHolder : NSObject
@property (nonatomic, readonly) CHIP_ERROR error;
@end

@implementation MTRErrorHolder

- (instancetype)initWithError:(CHIP_ERROR)error
{
    if (!(self = [super init])) {
        return nil;
    }

    _error = error;
    return self;
}

@end

CHIP_ERROR MTRErrorToCHIPErrorCode(NSError * error)
{
    if (error == nil) {
        return CHIP_NO_ERROR;
    }

    if (error.domain == MTRInteractionErrorDomain) {
        chip::app::StatusIB status(static_cast<chip::Protocols::InteractionModel::Status>(error.code));
        if (error.userInfo != nil && error.userInfo[@"clusterStatus"] != nil) {
            status.mClusterStatus.Emplace([error.userInfo[@"clusterStatus"] unsignedCharValue]);
        }
        return status.ToChipError();
    }

    if (error.domain != MTRErrorDomain) {
        return CHIP_ERROR_INTERNAL;
    }

    if (error.userInfo != nil) {
        id underlyingError = error.userInfo[@"underlyingError"];
        if (underlyingError != nil && [underlyingError isKindOfClass:[MTRErrorHolder class]]) {
            return ((MTRErrorHolder *) underlyingError).error;
        }
    }

    chip::ChipError::StorageType code;
    switch (error.code) {
    case MTRErrorCodeInvalidStringLength:
        code = CHIP_ERROR_INVALID_STRING_LENGTH.AsInteger();
        break;
    case MTRErrorCodeInvalidIntegerValue:
        code = CHIP_ERROR_INVALID_INTEGER_VALUE.AsInteger();
        break;
    case MTRErrorCodeInvalidArgument:
        code = CHIP_ERROR_INVALID_ARGUMENT.AsInteger();
        break;
    case MTRErrorCodeInvalidMessageLength:
        code = CHIP_ERROR_INVALID_MESSAGE_LENGTH.AsInteger();
        break;
    case MTRErrorCodeInvalidState:
        code = CHIP_ERROR_INCORRECT_STATE.AsInteger();
        break;
    case MTRErrorCodeIntegrityCheckFailed:
        code = CHIP_ERROR_INTEGRITY_CHECK_FAILED.AsInteger();
        break;
    case MTRErrorCodeTimeout:
        code = CHIP_ERROR_TIMEOUT.AsInteger();
        break;
    case MTRErrorCodeGeneralError: {
        if (error.userInfo != nil && error.userInfo[@"errorCode"] != nil) {
            code = static_cast<decltype(code)>([error.userInfo[@"errorCode"] unsignedLongValue]);
            break;
        }
        // Weird error we did not create.  Fall through.
    default:
        code = CHIP_ERROR_INTERNAL.AsInteger();
        break;
    }
    }

    return chip::ChipError(code);
}

// Convert TLV data into NSObject
id NSObjectFromCHIPTLV(chip::TLV::TLVReader * data)
{
    chip::TLV::TLVType dataTLVType = data->GetType();
    switch (dataTLVType) {
    case chip::TLV::kTLVType_SignedInteger: {
        int64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV signed integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRSignedIntegerValueType, MTRTypeKey, [NSNumber numberWithLongLong:val],
                             MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV unsigned integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRUnsignedIntegerValueType, MTRTypeKey,
                             [NSNumber numberWithUnsignedLongLong:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV boolean decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary
            dictionaryWithObjectsAndKeys:MTRBooleanValueType, MTRTypeKey, [NSNumber numberWithBool:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        // Try float first
        float floatValue;
        CHIP_ERROR err = data->Get(floatValue);
        if (err == CHIP_NO_ERROR) {
            return @ { MTRTypeKey : MTRFloatValueType, MTRValueKey : [NSNumber numberWithFloat:floatValue] };
        }
        double val;
        err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV floating point decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary
            dictionaryWithObjectsAndKeys:MTRDoubleValueType, MTRTypeKey, [NSNumber numberWithDouble:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_UTF8String: {
        uint32_t len = data->GetLength();
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV UTF8String decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRUTF8StringValueType, MTRTypeKey,
                             [[NSString alloc] initWithBytes:ptr length:len encoding:NSUTF8StringEncoding], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_ByteString: {
        uint32_t len = data->GetLength();
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV ByteString decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTROctetStringValueType, MTRTypeKey,
                             [NSData dataWithBytes:ptr length:len], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_Null: {
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRNullValueType, MTRTypeKey, nil];
    }
    case chip::TLV::kTLVType_Structure:
    case chip::TLV::kTLVType_Array: {
        NSString * typeName;
        switch (dataTLVType) {
        case chip::TLV::kTLVType_Structure:
            typeName = MTRStructureValueType;
            break;
        case chip::TLV::kTLVType_Array:
            typeName = MTRArrayValueType;
            break;
        default:
            typeName = @"Unsupported";
            break;
        }
        chip::TLV::TLVType tlvType;
        CHIP_ERROR err = data->EnterContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV container entering failed", chip::ErrorStr(err));
            return nil;
        }
        NSMutableArray * array = [[NSMutableArray alloc] init];
        while ((err = data->Next()) == CHIP_NO_ERROR) {
            chip::TLV::Tag tag = data->GetTag();
            id value = NSObjectFromCHIPTLV(data);
            if (value == nullptr) {
                MTR_LOG_ERROR("Error when decoding TLV container");
                return nil;
            }
            NSMutableDictionary * arrayElement = [NSMutableDictionary dictionary];
            [arrayElement setObject:value forKey:MTRDataKey];
            if (dataTLVType == chip::TLV::kTLVType_Structure) {
                [arrayElement setObject:[NSNumber numberWithUnsignedLong:TagNumFromTag(tag)] forKey:MTRContextTagKey];
            }
            [array addObject:arrayElement];
        }
        if (err != CHIP_END_OF_TLV) {
            MTR_LOG_ERROR("Error(%s): TLV container decoding failed", chip::ErrorStr(err));
            return nil;
        }
        err = data->ExitContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV container exiting failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:typeName, MTRTypeKey, array, MTRValueKey, nil];
    }
    default:
        MTR_LOG_ERROR("Error: Unsupported TLV type for conversion: %u", (unsigned) data->GetType());
        return nil;
    }
}
