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

#import "MCCommand.h"

#import "MCCastingApp.h"
#import "MCErrorUtils.h"
#include "core/Command.h"
#include <lib/core/CHIPError.h>

#import <Foundation/Foundation.h>
#include <any>
#include <functional>
#include <memory>

#ifndef MCCommand_Internal_h
#define MCCommand_Internal_h

@interface MCCommand <RequestType, ResponseType>()

@property (nonatomic, readwrite) void * _Nonnull cppCommand;

- (instancetype _Nonnull)initWithCppCommand:(void * _Nonnull)cppCommand;

- (std::any)getCppRequestFromObjC:(id _Nonnull)objcRequest;

- (id _Nullable)getObjCResponseFromCpp:(std::any)cppResponse;
@end

template <typename Type>
class MCCommandTemplate {
public:
    MCCommandTemplate(void * _Nonnull cppCommand,
        std::function<std::any(id _Nonnull)> getCppRequestFromObjCFn,
        std::function<id(std::any)> getObjCResponseFromCppFn)
    {
        mCppCommand = cppCommand;
        mGetCppRequestFromObjCFn = getCppRequestFromObjCFn;
        mGetObjCResponseFromCppFn = getObjCResponseFromCppFn;
    }

    void invoke(id _Nonnull request, void * _Nullable context, std::function<void(void * _Nullable, NSError * _Nullable, id _Nullable)> completion, NSNumber * _Nullable timedInvokeTimeoutMs)
    {
        dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue], clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
        dispatch_sync(workQueue, ^{
            ChipLogProgress(AppServer, "<MCCommandTemplate> converting 'request' from ObjC to Cpp");
            std::shared_ptr<Type> cppRequest = nil;
            std::any anyRequest = mGetCppRequestFromObjCFn(request);
            if (anyRequest.type() == typeid(std::shared_ptr<Type>)) {
                cppRequest = std::any_cast<std::shared_ptr<Type>>(anyRequest);
            }
            if (cppRequest == nil) {
                dispatch_async(clientQueue, ^{
                    completion(context, [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT], nil);
                });
                return;
            }

            matter::casting::core::Command<Type> * command = static_cast<matter::casting::core::Command<Type> *>(mCppCommand);
            command->Invoke(
                *cppRequest, context,
                [clientQueue, completion, this](void * context, const typename Type::ResponseType & response) {
                    ChipLogProgress(AppServer, "<MCCommandTemplate> converting 'response' from Cpp to ObjC");
                    // Get cpp response here.
                    id objCResponse = mGetObjCResponseFromCppFn(std::any(std::make_shared<const typename Type::ResponseType>(response)));
                    dispatch_async(clientQueue, ^{
                        completion(context, nil, objCResponse);
                    });
                },
                [clientQueue, completion](void * context, CHIP_ERROR err) {
                    dispatch_async(clientQueue, ^{
                        completion(context, [MCErrorUtils NSErrorFromChipError:err], nil);
                    });
                },
                timedInvokeTimeoutMs != nil ? chip::MakeOptional(timedInvokeTimeoutMs.intValue) : chip::NullOptional);
        });
    }

private:
    void * _Nonnull mCppCommand;
    std::function<std::any(id _Nonnull)> mGetCppRequestFromObjCFn;
    std::function<id(std::any)> mGetObjCResponseFromCppFn;
};

#endif /* MCCommand_Internal_h */
