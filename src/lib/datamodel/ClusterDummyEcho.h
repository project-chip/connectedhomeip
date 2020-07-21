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
 *      This file contains definitions for a dummy CHIP Cluster 'Echo' that can be used for
 *      end-to-end testing.
 *
 */

#ifndef CHIPCLUSTER_ECHO_H_
#define CHIPCLUSTER_ECHO_H_

#include <datamodel/Cluster.h>

namespace chip {
namespace DataModel {

/* Cluster ID */
static const ClusterId_t kClusterIdDummyEcho = 0xfc00;

/* Attribute IDs */
static const AttributeId_t kAttributeIdDummyEcho = 0x0000;

/* Command IDs */
static const CommandId_t kDummyEchoCmdIdEchoRequest  = 0x00;
static const CommandId_t kDummyEchoCmdIdEchoResponse = 0x00;

static inline void ClusterDummyEchoGenerateCommand(Command & cmd, CommandId_t cmdId, uint16_t endpointId)
{
    cmd.mEndpointId = endpointId;
    cmd.mType       = kCmdTypeCluster;
    cmd.mClusterId  = kClusterIdDummyEcho;
    cmd.mId         = cmdId;
    cmd.mDirection  = cmdId == kDummyEchoCmdIdEchoRequest ? kCmdDirectionClientToServer : kCmdDirectionServerToClient;
    cmd.StartEncode();
    cmd.EndEncode();
}

static inline void ClusterDummyEchoEncodeEchoRequest(Command & cmd, uint16_t endpointId)
{
    ClusterDummyEchoGenerateCommand(cmd, kDummyEchoCmdIdEchoRequest, endpointId);
}

static inline void ClusterDummyEchoEncodeEchoResponse(Command & cmd, uint16_t endpointId)
{
    ClusterDummyEchoGenerateCommand(cmd, kDummyEchoCmdIdEchoResponse, endpointId);
};

/**
 * @brief
 *   This class implements the DummyEcho cluster as defined in the CHIP specification.
 */
class ClusterDummyEcho : public Cluster
{
private:
    /* Just a placeholder attribute that is never assigned to */
    AttributeSimple<bool> mDummyEcho;

public:
    ClusterDummyEcho() : Cluster(kClusterIdDummyEcho), mDummyEcho(kAttributeIdDummyEcho) { AddAttribute(&mDummyEcho); }

    virtual CHIP_ERROR HandleCommandEchoRequest(Command & cmd)
    {
        /* TODO The endpoint is fake here */
        ClusterDummyEchoEncodeEchoResponse(cmd, 1);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Handle commands for the Cluster DummyEcho. This is already handled in the ClusterDummyEcho
     *   class. Applications may choose to override this handling if required.
     *
     * @param cmd the command to handle
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    virtual CHIP_ERROR HandleCommand(Command & cmd)
    {
        switch (cmd.mId)
        {
        case kDummyEchoCmdIdEchoRequest:
            return HandleCommandEchoRequest(cmd);
        default:
            /* Unsupported */
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTER_ECHO_H_ */
