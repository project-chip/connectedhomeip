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

#import "MTREventTLVValueDecoder_Internal.h"

#import "MTRStructsObjc.h"
#import "MTRStructsObjc_Private.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app/EventHeader.h>
#include <app/EventLoggingTypes.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;

id _Nullable MTRPrivateDecodeEventPayload(const ConcreteEventPath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    switch (aPath.mClusterId) {
    default: {
        break;
    }
    }
    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
