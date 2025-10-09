/**
 *    Copyright (c) 2021-2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app/ClusterStateCache.h>
#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

NS_ASSUME_NONNULL_BEGIN

// Decodes an attribute value TLV into a typed ObjC value (see MTRStructsObjc.h)
id _Nullable MTRDecodeAttributeValue(const chip::app::ConcreteAttributePath & aPath, chip::TLV::TLVReader & aReader,
                                     CHIP_ERROR * aError);

// Wrapper around the precending function that reads the attribute from a ClusterStateCache.
id _Nullable MTRDecodeAttributeValue(const chip::app::ConcreteAttributePath & aPath, const chip::app::ClusterStateCache & aCache,
                                     CHIP_ERROR * aError);

NS_ASSUME_NONNULL_END
