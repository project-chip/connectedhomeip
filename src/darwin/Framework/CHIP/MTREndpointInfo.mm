/**
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

#import "MTREndpointInfo_Internal.h"

#import "MTRAttributeTLVValueDecoder_Internal.h"
#import "MTRDeviceTypeRevision.h"
#import "MTRLogging_Internal.h"
#import "MTRStructsObjc.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <deque>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

enum class EndpointMark : uint8_t {
    NotVisited = 0,
    Visiting,
    Visited,
    ParentAssigned = NotVisited, // != Visited
};

MTR_DIRECT_MEMBERS
@implementation MTREndpointInfo {
    EndpointId _endpointID;
    EndpointMark _mark; // used by populateChildrenForEndpoints:
}

- (instancetype)initWithEndpointID:(NSNumber *)endpointID
                       deviceTypes:(NSArray<MTRDeviceTypeRevision *> *)deviceTypes
                         partsList:(NSArray<NSNumber *> *)partsList
{
    self = [super init];
    _endpointID = endpointID.unsignedShortValue;
    _deviceTypes = [deviceTypes copy];
    _partsList = [partsList copy];
    _children = @[];
    return self;
}

- (NSNumber *)endpointID
{
    return @(_endpointID);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %u>", self.class, _endpointID];
}

+ (BOOL)populateChildrenForEndpoints:(NSDictionary<NSNumber *, MTREndpointInfo *> *)endpoints
{
    // Populate the child list of each endpoint, ensuring no cycles (these are disallowed
    // by the spec, but we need to ensure we don't create a retain cycle even under invalid
    // input). Conservatively assume all endpoints use the Full-Family Pattern.
    // Refer to "Endpoint Composition" in the Matter specification for details.
    MTREndpointInfo * root = endpoints[@0];
    if (root == nil) {
        MTR_LOG_ERROR("Missing root endpoint, not populating endpoint hierarchy");
        return NO;
    }

    // Perform a depth-first search with an explicit stack and create a list of endpoint
    // IDs in reverse topological order. The _parentID field is used mark traversal states.
    BOOL valid = YES;
    std::deque<EndpointId> deque; // stack followed by sorted list
    deque.emplace_front(root->_endpointID);
    for (;;) {
        EndpointId endpointID = deque.front();
        MTREndpointInfo * endpoint = endpoints[@(endpointID)];
        if (endpoint->_mark == EndpointMark::NotVisited) {
            endpoint->_mark = EndpointMark::Visiting;
            for (NSNumber * partNumber in endpoint->_partsList) {
                MTREndpointInfo * descendant = endpoints[partNumber];
                if (!descendant) {
                    MTR_LOG_ERROR("Warning: PartsList of endpoint %u references non-existant endpoint %u",
                        endpointID, partNumber.unsignedShortValue);
                    valid = NO;
                } else if (descendant->_mark == EndpointMark::NotVisited) {
                    deque.emplace_front(descendant->_endpointID);
                } else if (descendant->_mark == EndpointMark::Visiting) {
                    MTR_LOG_ERROR("Warning: Cyclic endpoint composition involving endpoints %u and %u",
                        descendant->_endpointID, endpointID);
                    valid = NO;
                }
            }
        } else if (endpoint->_mark == EndpointMark::Visiting) {
            endpoint->_mark = EndpointMark::Visited;
            deque.pop_front(); // remove from stack
            deque.emplace_back(endpointID); // add to sorted list
            if (endpointID == root->_endpointID) {
                break; // visited the root, DFS traversal done
            }
        } else /* endpoint->_mark == EndpointMark::Visited */ {
            deque.pop_front(); // nothing else to do
        }
    }
    if (deque.size() != endpoints.count) {
        MTR_LOG_ERROR("Warning: Not all endpoints are descendants of the root endpoint");
        valid = NO;
    }

    // Now iterate over the endpoints in reverse topological order, i.e. bottom up. This means
    // that we will visit children before parents, so the first time we see an endpoint in a
    // PartsList we can assign it as a child of the endpoint we're processing, and we can be sure
    // that this is the closest parent, not some higher ancestor using the Full-Family Pattern.
    NSMutableArray<MTREndpointInfo *> * children = [[NSMutableArray alloc] init];
    while (!deque.empty()) {
        EndpointId endpointID = deque.front();
        MTREndpointInfo * endpoint = endpoints[@(endpointID)];
        deque.pop_front();

        if (endpoint->_mark == EndpointMark::ParentAssigned) {
            continue; // This endpoint is part of a cycle, don't populate its children.
        }

        [children removeAllObjects];
        for (NSNumber * partNumber in endpoint->_partsList) {
            MTREndpointInfo * descendant = endpoints[partNumber];
            if (descendant != nil && descendant->_mark != EndpointMark::ParentAssigned) {
                descendant->_mark = EndpointMark::ParentAssigned;
                [children addObject:descendant];
            }
        }
        endpoint->_children = [children copy];
    }
    root->_mark = EndpointMark::ParentAssigned;
    return valid;
}

+ (NSDictionary<NSNumber *, MTREndpointInfo *> *)endpointsFromAttributeCache:(const ClusterStateCache *)cache
{
    VerifyOrReturnValue(cache != nullptr, nil);
    using namespace Descriptor::Attributes;

    NSMutableDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [[NSMutableDictionary alloc] init];
    cache->ForEachAttribute(Descriptor::Id, [&](const ConcreteAttributePath & path) -> CHIP_ERROR {
        VerifyOrReturnError(path.mAttributeId == DeviceTypeList::Id, CHIP_NO_ERROR);

        CHIP_ERROR err = CHIP_NO_ERROR;
        NSArray<MTRDescriptorClusterDeviceTypeStruct *> * deviceTypeList = MTRDecodeAttributeValue(path, *cache, &err);
        if (!deviceTypeList) {
            MTR_LOG_ERROR("Ignoring endpoint %u, failed to parse DeviceTypeList: %" CHIP_ERROR_FORMAT, path.mEndpointId, err.Format());
            return CHIP_NO_ERROR;
        }

        NSMutableArray * deviceTypes = [[NSMutableArray alloc] initWithCapacity:deviceTypeList.count];
        for (MTRDescriptorClusterDeviceTypeStruct * deviceTypeStruct in deviceTypeList) {
            MTRDeviceTypeRevision * type = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeStruct:deviceTypeStruct];
            if (!type) {
                MTR_LOG_ERROR("Ignoring invalid device type 0x%x rev %u for endpoint %u",
                    deviceTypeStruct.deviceType.unsignedIntValue, deviceTypeStruct.revision.unsignedShortValue,
                    path.mEndpointId);
                continue;
            }
            [deviceTypes addObject:type];
        }
        if (!deviceTypes) {
            MTR_LOG_ERROR("Ignoring endpoint %u, no device types", path.mEndpointId);
            return CHIP_NO_ERROR;
        }

        ConcreteAttributePath partsListPath(path.mEndpointId, path.mClusterId, PartsList::Id);
        NSArray<NSNumber *> * partsList = MTRDecodeAttributeValue(partsListPath, *cache, &err);
        if (!partsList) {
            MTR_LOG_ERROR("Ignoring PartsList for endpoint %u: %" CHIP_ERROR_FORMAT, path.mEndpointId, err.Format());
            partsList = @[];
        }

        MTREndpointInfo * endpoint = [[MTREndpointInfo alloc] initWithEndpointID:@(path.mEndpointId)
                                                                     deviceTypes:deviceTypes
                                                                       partsList:partsList];
        endpoints[endpoint.endpointID] = endpoint;
        return CHIP_NO_ERROR;
    });

    if (endpoints.count > 0) {
        [self populateChildrenForEndpoints:endpoints];
    }
    return [endpoints copy];
}

+ (Span<const AttributePathParams>)requiredAttributePaths
{
    using namespace Descriptor::Attributes;
    static constexpr AttributePathParams kPaths[] = {
        AttributePathParams(Descriptor::Id, DeviceTypeList::Id),
        AttributePathParams(Descriptor::Id, PartsList::Id),
    };
    return Span<const AttributePathParams>(kPaths);
}

@end
