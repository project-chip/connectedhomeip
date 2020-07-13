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
 *      This file contains class for Commands
 *
 */

#ifndef CHIPCOMMAND_H_
#define CHIPCOMMAND_H_

namespace chip {
namespace DataModel {

enum CmdTypes
{
    kCmdTypeUnknown = 0,
    kCmdTypeGlobal  = 1,
    kCmdTypeCluster = 2,
    kCmdTypeMfg     = 3,
};

enum CmdDirection : uint8_t
{
    kCmdDirectionClientToServer = 0,
    kCmdDirectionServerToClient = 1,
};

class Command
{
public:
    /* The type of command, global, cluster-specific or manufacturer-specific */
    CmdTypes mType;
    /* The command identifier */
    uint16_t mId;
    /* The endpoint this command is for */
    uint8_t mEndpointId;
    /* The direction of the command */
    CmdDirection mDirection;
    union {
        /* The ClusterId, if this is cluster-specific command */
        uint16_t mClusterId;
        /* The MfgCode, if this is manufacturer-specific command */
        uint16_t mMfgCode;
    };
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCOMMAND_H_ */
