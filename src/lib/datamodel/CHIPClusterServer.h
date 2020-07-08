/*
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

/**
 *    @file
 *      This file contains the ClusterServer class. The ClusterServer
 *      maintains all the cluster servers that this devices exposes.
 *
 */

#ifndef CHIPCLUSTERSERVER_H_
#define CHIPCLUSTERSERVER_H_

#include <datamodel/CHIPEndPoint.h>

namespace chip {
namespace DataModel {

/* TODO: To be converted to a template version or Kconfig later on */
static const uint8_t kMaxEndPointPerServer = 5;
/* TODO: If endpoint numbers range up to 256, it may be better not to use array index as the endpoint id */
/* Skip the reserved endPointId */
static const uint8_t kEndPointIdStart = 1;
class ClusterServer
{
public:
    EndPoint * mEndPoints[kMaxEndPointPerServer];

    ClusterServer(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion) : mEndPoints()
    {
        mEndPoints[kEndPointIdStart] = new EndPoint(ZCLVersion, applicationVersion, stackVersion, HWVersion);
    }

    virtual ~ClusterServer()
    {
        for (int i = kEndPointIdStart; i < kMaxEndPointPerServer; i++)
        {
            if (mEndPoints[i] != nullptr)
            {
                delete mEndPoints[i];
                mEndPoints[i] = nullptr;
            }
        }
    }

    /* By default always add to the kEndPointIdStartth endpoint, for simplicity */
    int AddCluster(BaseCluster * cluster)
    {
        if (!cluster)
        {
            return FAIL;
        }

        return mEndPoints[kEndPointIdStart]->AddCluster(cluster);
    }

    int AddEndPoint(EndPoint * endPoint)
    {
        for (int i = kEndPointIdStart; i < kMaxEndPointPerServer; i++)
        {
            if (mEndPoints[i] == nullptr)
            {
                mEndPoints[i] = endPoint;
                return SUCCESS;
            }
        }
        return FAIL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERSERVER_H_ */
