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
 *      This file contains the ZCLCommand class
 *
 */

#ifndef CHIPZCLCOMMAND_H_
#define CHIPZCLCOMMAND_H_

#include <core/CHIPError.h>
#include <datamodel/Command.h>
#include <system/SystemPacketBuffer.h>

extern "C" {
#include "chip-zcl/chip-zcl-codec.h"
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-cluster-id.h"
#include "gen/gen-types.h"
}

namespace chip {
namespace DataModel {

/* TODO: Move this into a directory of its own, doesn't belong in lib/datamodel */
class ZCLCommand : public Command
{
public:
    ChipZclCodec_t mCodec;
    ChipZclBuffer_t * mBuffer;

    ZCLCommand(System::PacketBuffer * buffer): mBuffer((ChipZclBuffer_t *)buffer) { }

    void StartDecode(void)
    {
        uint8_t mask = 0;
        chipZclCodecDecodeStart(&mCodec, mBuffer);
        chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &mask, sizeof(mask), NULL);
        chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mEndpointId), sizeof(mEndpointId), NULL);

        chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mId), sizeof(mId), NULL);
        chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mDirection), sizeof(mDirection), NULL);
        mType = kCmdTypeGlobal;
        if (mask & 0x01)
        {
            mType = kCmdTypeCluster;
            chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mClusterId), sizeof(mClusterId), NULL);
        }
        if (mask & 0x02)
        {
            mType = kCmdTypeMfg;
            chipZclCodecDecode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mMfgCode), sizeof(mMfgCode), NULL);
        }
    }

    void EndDecode(void)
    {
        chipZclCodecDecodeEnd(&mCodec);
    }

    void StartEncode(void)
    {
        uint8_t mask = 0;

        chipZclCodecEncodeStart(&mCodec, mBuffer);
        if (mType == kCmdTypeCluster)
        {
            mask |= 0x01;
        }
        if (mType == kCmdTypeMfg)
        {
            mask |= 0x02;
        }
        chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &mask, sizeof(mask));
        chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mEndpointId), sizeof(mEndpointId));
        chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mId), sizeof(mId));
        chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mDirection), sizeof(mDirection));
        if (mask & 0x01)
        {
            chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mClusterId), sizeof(mClusterId));
        }
        if (mask & 0x02)
        {
            chipZclCodecEncode(&mCodec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(mMfgCode), sizeof(mMfgCode));
        }
        
    }

    void EndEncode(void)
    {
        chipZclCodecEncodeEnd(&mCodec);
    }

};

} // namespace DataModel
} // namespace chip

#endif /* CHIPZCLCOMMAND_H_ */
