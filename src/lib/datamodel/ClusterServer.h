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
 *      This file contains the Cluster Server class. The Cluster Server maintains all the cluster
 *      servers that this devices exposes.
 *
 */

#ifndef CHIPCLUSTERSERVER_H_
#define CHIPCLUSTERSERVER_H_

#include <datamodel/EndPoint.h>

namespace chip {
namespace DataModel {

/* TODO: To be converted to a template version or Kconfig later on */
static const uint8_t kMaxEndPointPerServer = 5;
/* TODO: If endpoint numbers range up to 256, it may be better not to use array index as the endpoint id */
/* Skip the reserved endPointId */
static const uint8_t kEndPointIdStart = 1;

/**
 * @brief
 *   This class implements the Cluster Server that maintains all the server endpoints of this
 *   device.
 */
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

    /**
     * @brief
     *   Add clusters to endpointId 1 of this Cluster Server
     *
     * @param cluster Pointer to the cluster object being added
     */
    CHIP_ERROR AddCluster(Cluster * cluster)
    {
        if (!cluster)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return mEndPoints[kEndPointIdStart]->AddCluster(cluster);
    }

    /**
     * @brief
     *   Add a new endpoint to this Cluster Server
     *
     * @param cluster Pointer to the endpoint object being added
     */
    CHIP_ERROR AddEndPoint(EndPoint * endPoint)
    {
        for (int i = kEndPointIdStart; i < kMaxEndPointPerServer; i++)
        {
            if (mEndPoints[i] == nullptr)
            {
                mEndPoints[i] = endPoint;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERSERVER_H_ */
