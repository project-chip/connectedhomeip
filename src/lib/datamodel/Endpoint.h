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
 *      This file contains the Endpoint class
 *
 */

#ifndef CHIPENDPOINT_H_
#define CHIPENDPOINT_H_

#include <datamodel/Attribute.h>
#include <datamodel/ClusterBasic.h>
#include <datamodel/Deque.h>

namespace chip {
namespace DataModel {

/* The ZCL/CHIP Version that is supported - this is arbitrary for now */
static const uint8_t kZCLVersion = 5;
/* The current stack version */
static const uint8_t kstackVersion = 10;

/**
 * @brief
 *   This class implements the endpoint that maintains all the clusters supported by this endpoint.
 */
class Endpoint : public Deque<Endpoint>
{

public:
    ClusterBasic mClusters; // head pointer is a BasicCluster

    Endpoint(uint8_t applicationVersion, uint8_t HWVersion) :
        Deque(this), mClusters(kZCLVersion, applicationVersion, kstackVersion, HWVersion)
    {}

    virtual ~Endpoint() {}

    /**
     * @brief
     *   Add clusters to this endpoint
     *
     * @param cluster Pointer to the cluster object being added
     */
    CHIP_ERROR AddCluster(Cluster * cluster)
    {
        mClusters.Insert(cluster);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Get pointer to the cluster object
     *
     * @param clusterId the cluster identifer that we are looking for
     */
    Cluster * GetCluster(uint8_t clusterId)
    {
        return mClusters.Find([clusterId](Cluster * item) -> bool { return (item->mClusterId == clusterId); });
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPENDPOINT_H_ */
