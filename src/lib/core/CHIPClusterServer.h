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

#include <lib/core/CHIPEndPoint.h>

namespace chip {
namespace DataModel {

/* TODO: To be converted to a template version or Kconfig later on */
static const uint8_t kMaxEndPointPerServer = 5;
class CHIPClusterServer
{
public:
    CHIPEndPoint * mEndPoints[kMaxEndPointPerServer];

    CHIPClusterServer(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion) : mEndPoints()
    {
        mEndPoints[0] = new CHIPEndPoint(ZCLVersion, applicationVersion, stackVersion, HWVersion);
    }

    virtual ~CHIPClusterServer()
    {
        for (int i = 1; i < kMaxEndPointPerServer; i++)
        {
            if (mEndPoints[i] != nullptr)
            {
                delete mEndPoints[i];
                mEndPoints[i] = nullptr;
            }
        }
    }

    /* By default always add to the 0th endpoint, for simplicity */
    int AddCluster(CHIPBaseCluster * cluster)
    {
        if (!cluster)
        {
            return FAIL;
        }

        return mEndPoints[0]->AddCluster(cluster);
    }

    int AddEndPoint(CHIPEndPoint * endPoint)
    {
        for (int i = 0; i < kMaxEndPointPerServer; i++)
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
