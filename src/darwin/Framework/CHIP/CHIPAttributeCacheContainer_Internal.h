/**
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

#import "CHIPAttributeCacheContainer.h"
#import "CHIPDeviceControllerOverXPC.h"

#include <app/AttributeCache.h>

NS_ASSUME_NONNULL_BEGIN

class ContainerAttributeCacheCallback : public chip::app::AttributeCache::Callback {
public:
    void SetContainer(CHIPAttributeCacheContainer * container) { attributeCacheContainer = container; }

    void OnDone() override;

private:
    __weak CHIPAttributeCacheContainer * _Nullable attributeCacheContainer;
};

@interface CHIPAttributeCacheContainer ()

@property (nonatomic, readwrite) chip::app::AttributeCache * _Nullable cppAttributeCache;
@property (nonatomic, readwrite) chip::app::ReadClient * _Nullable cppReadClient;
@property (nonatomic, readwrite) ContainerAttributeCacheCallback * _Nullable attributeCacheCallback;
@property (nonatomic, readwrite) uint64_t deviceId;
@property (atomic, readwrite, weak) CHIPDeviceControllerOverXPC * _Nullable xpcDeviceController;

@end

NS_ASSUME_NONNULL_END
