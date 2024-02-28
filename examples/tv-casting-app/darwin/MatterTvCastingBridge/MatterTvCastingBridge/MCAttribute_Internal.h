/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MCAttribute.h"

#import "MCCastingApp.h"
#import "MCErrorUtils.h"
#include "core/Attribute.h"
#include <lib/core/CHIPError.h>

#import <Foundation/Foundation.h>
#include <any>
#include <functional>
#include <memory>

#ifndef MCAttribute_Internal_h
#define MCAttribute_Internal_h

@interface MCAttribute <ObjectType>()

@property (nonatomic, readwrite) void * _Nonnull cppAttribute;

- (instancetype _Nonnull)initWithCppAttribute:(void * _Nonnull)cppAttribute;

- (id _Nullable)getObjCTypeFromCpp:(std::any)cppValue errorCode:(CHIP_ERROR * _Nonnull)aError;

@end

template <typename TypeInfo>
class MCAttributeTemplate {
public:
    MCAttributeTemplate(void * _Nonnull cppAttribute,
        std::function<id(std::any, CHIP_ERROR * _Nonnull)> getObjCTypeFromCppFn)
    {
        mCppAttribute = cppAttribute;
        mGetObjCTypeFromCppFn = getObjCTypeFromCppFn;
    }

    void read(void * _Nullable context, std::function<void(void * _Nullable, id _Nullable, id _Nullable, NSError * _Nullable)> completion)
    {
        dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue], clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
        dispatch_sync(workQueue, ^{
            matter::casting::core::Attribute<TypeInfo> * attribute = static_cast<matter::casting::core::Attribute<TypeInfo> *>(mCppAttribute);

            attribute->Read(
                context,
                [clientQueue, completion, this](void * context, chip::Optional<typename TypeInfo::DecodableType> before, typename TypeInfo::DecodableArgType after) {
                    NSNumber *objCBefore = nil, *objCAfter = nil;
                    CHIP_ERROR errBefore = CHIP_NO_ERROR, errAfter = CHIP_NO_ERROR;
                    if (before.HasValue()) {
                        ChipLogProgress(AppServer, "<MCAttributeTemplate> converting 'before' value from Cpp to ObjC");
                        std::any anyBefore = std::any(std::make_shared<typename TypeInfo::DecodableType>(before.Value()));
                        objCBefore = mGetObjCTypeFromCppFn(anyBefore, &errBefore);
                    }
                    ChipLogProgress(AppServer, "<MCAttributeTemplate> converting 'after' value from Cpp to ObjC");
                    std::any anyAfter = std::any(std::make_shared<typename TypeInfo::DecodableType>(after));
                    objCAfter = mGetObjCTypeFromCppFn(anyAfter, &errAfter);
                    dispatch_async(clientQueue, ^{
                        if (errBefore == CHIP_NO_ERROR && errAfter == CHIP_NO_ERROR) {
                            completion(context, objCBefore, objCAfter, nil);
                        } else {
                            completion(context, nil, nil, [MCErrorUtils NSErrorFromChipError:(errAfter != CHIP_NO_ERROR ? errAfter : errBefore)]);
                        }
                    });
                },
                [clientQueue, completion](void * context, CHIP_ERROR error) {
                    dispatch_async(clientQueue, ^{
                        completion(context, nil, nil, [MCErrorUtils NSErrorFromChipError:error]);
                    });
                });
        });
    }

    void subscribe(void * _Nullable context, std::function<void(void * _Nullable, id _Nullable, id _Nullable, NSError * _Nullable)> completion,
        NSNumber * _Nonnull minInterval, NSNumber * _Nonnull maxInterval)
    {
        dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue], clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
        dispatch_sync(workQueue, ^{
            matter::casting::core::Attribute<TypeInfo> * attribute = static_cast<matter::casting::core::Attribute<TypeInfo> *>(mCppAttribute);

            attribute->Subscribe(
                context,
                [clientQueue, completion, this](void * context, chip::Optional<typename TypeInfo::DecodableType> before, typename TypeInfo::DecodableArgType after) {
                    NSNumber *objCBefore = nil, *objCAfter = nil;
                    CHIP_ERROR errBefore = CHIP_NO_ERROR, errAfter = CHIP_NO_ERROR;
                    if (before.HasValue()) {
                        ChipLogProgress(AppServer, "<MCAttributeTemplate> converting 'before' value from Cpp to ObjC");
                        std::any anyBefore = std::any(std::make_shared<typename TypeInfo::DecodableType>(before.Value()));
                        objCBefore = mGetObjCTypeFromCppFn(anyBefore, &errBefore);
                    }
                    ChipLogProgress(AppServer, "<MCAttributeTemplate> converting 'after' value from Cpp to ObjC");
                    std::any anyAfter = std::any(std::make_shared<typename TypeInfo::DecodableType>(after));
                    objCAfter = mGetObjCTypeFromCppFn(anyAfter, &errAfter);
                    dispatch_async(clientQueue, ^{
                        if (errBefore == CHIP_NO_ERROR && errAfter == CHIP_NO_ERROR) {
                            completion(context, objCBefore, objCAfter, nil);
                        } else {
                            completion(context, nil, nil, [MCErrorUtils NSErrorFromChipError:(errAfter != CHIP_NO_ERROR ? errAfter : errBefore)]);
                        }
                    });
                },
                [clientQueue, completion](void * context, CHIP_ERROR error) {
                    dispatch_async(clientQueue, ^{
                        completion(context, nil, nil, [MCErrorUtils NSErrorFromChipError:error]);
                    });
                },
                minInterval.intValue,
                maxInterval.intValue);
        });
    }

private:
    void * _Nonnull mCppAttribute;
    std::function<id(std::any, CHIP_ERROR * _Nonnull)> mGetObjCTypeFromCppFn;
};
#endif /* MCAttribute_Internal_h */
