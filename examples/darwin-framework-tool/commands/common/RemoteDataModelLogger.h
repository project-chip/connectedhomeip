/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import <Matter/Matter.h>

#include <lib/core/CHIPError.h>

class RemoteDataModelLoggerDelegate {
public:
    CHIP_ERROR virtual LogJSON(const char *) = 0;
    virtual ~RemoteDataModelLoggerDelegate() {};
};

namespace RemoteDataModelLogger {
CHIP_ERROR LogAttributeAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId, id result);
CHIP_ERROR LogCommandAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id result);
CHIP_ERROR LogAttributeErrorAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId, NSError * error);
CHIP_ERROR LogCommandErrorAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, NSError * error);
CHIP_ERROR LogGetCommissionerNodeId(NSNumber * nodeId);
CHIP_ERROR LogBdxDownload(NSString * content, NSError * error);
void SetDelegate(RemoteDataModelLoggerDelegate * delegate);
}; // namespace RemoteDataModelLogger
