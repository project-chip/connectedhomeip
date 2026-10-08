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

#import <Foundation/Foundation.h>

#import "MTRBaseClusters_Private.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCallbackBridgeBase.h"
#import "MTRClusterStateCacheContainer_Internal.h"
#import "MTRCluster_Internal.h"
#import "MTRCommandPayloadsObjc.h"
#import "MTRDefines_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRStructsObjc.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/ListLargeSystemExtensions.h>
#include <app/util/im-client-callbacks.h>
#include <controller/CHIPCluster.h>
#include <platform/CHIPDeviceLayer.h>

#include <type_traits>

using chip::Callback::Callback;
using chip::Callback::Cancelable;
using namespace chip::app::Clusters;
using chip::Optional;
using chip::SessionHandle;
using chip::Messaging::ExchangeManager;
using chip::System::Clock::Seconds16;
using chip::System::Clock::Timeout;

typedef void (*DefaultSuccessCallbackType)(void *);

class MTRDefaultSuccessCallbackBridge : public MTRCallbackBridge<DefaultSuccessCallback> {
public:
    MTRDefaultSuccessCallbackBridge(dispatch_queue_t queue, ResponseHandler handler, MTRActionBlock action)
        : MTRCallbackBridge<DefaultSuccessCallback>(queue, handler, action, OnSuccessFn) { };

    static void OnSuccessFn(void * context)
    {
        DispatchSuccess(context, nil);
    }
};

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks): Linter is unable to locate the delete on these objects.
// Nothing here for now, but leaving this file in place in case we need to add
// something.

// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
