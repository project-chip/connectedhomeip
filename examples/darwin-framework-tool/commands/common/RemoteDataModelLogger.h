/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
void SetDelegate(RemoteDataModelLoggerDelegate * delegate);
}; // namespace RemoteDataModelLogger
