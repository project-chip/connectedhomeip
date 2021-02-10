/* See Project CHIP LICENSE file for licensing information. */

// bool emberAfProcessMessage(EmberApsFrame *apsFrame,
//                            EmberIncomingMessageType type,
//                            uint8_t *message,
//                            uint16_t msgLen,
//                            EmberNodeId source,
//                            InterPanHeader *interPanHeader)

#include <app/chip-zcl-zpro-codec.h>
#include <app/message-reader.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <stdio.h>
#include <string.h>
#include <support/logging/CHIPLogging.h>

extern "C" {

uint16_t extractApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * outApsFrame)
{
    if (buffer == nullptr || buf_length == 0 || outApsFrame == nullptr)
    {
        ChipLogError(Zcl, "Error extracting APS frame. invalid inputs");
        return 0;
    }

    chip::DataModelReader reader(buffer, buf_length);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Skip first byte, because that's the always-0 frame control.
    uint8_t ignored;
    err = reader.ReadOctet(&ignored)
              .ReadClusterId(&outApsFrame->clusterId)
              .ReadEndpointId(&outApsFrame->sourceEndpoint)
              .ReadEndpointId(&outApsFrame->destinationEndpoint)
              .Read16(&outApsFrame->options)
              .ReadGroupId(&outApsFrame->groupId)
              .ReadOctet(&outApsFrame->sequence)
              .ReadOctet(&outApsFrame->radius)
              .StatusCode();
    SuccessOrExit(err);

exit:
    return err == CHIP_NO_ERROR ? reader.OctetsRead() : 0;
}

void printApsFrame(EmberApsFrame * frame)
{
    ChipLogProgress(Zcl,
                    "\n<EmberApsFrame %p> clusterID %d, sourceEndpoint %d, destinationEndPoint %d, options %d, groupID %d, "
                    "sequence %d, radius %d\n",
                    frame, frame->clusterId, frame->sourceEndpoint, frame->destinationEndpoint, frame->options, frame->groupId,
                    frame->sequence, frame->radius);
}

uint16_t extractMessage(uint8_t * buffer, uint16_t buffer_length, uint8_t ** msg)
{
    // The message starts after the EmberApsFrame.
    uint16_t result = 0;
    EmberApsFrame frame;
    uint16_t apsFrameSize = extractApsFrame(buffer, buffer_length, &frame);
    if (msg && apsFrameSize > 0)
    {
        *msg   = buffer + apsFrameSize;
        result = static_cast<uint16_t>(buffer_length - apsFrameSize);
    }
    else if (msg)
    {
        *msg = nullptr;
    }
    return result;
}

} // extern C
