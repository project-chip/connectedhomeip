/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <transport/pushav-transport.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define AUD_FRAME_DURATION_48000 960
#define VID_FRAME_DURATION 66667 // Microseconds, for 30fps

#define IS_H264_FRAME_NALU_HEAD(frame)                                                                                             \
    (((frame)[0] == 0x00) && ((frame)[1] == 0x00) && (((frame)[2] == 0x01) || (((frame)[2] == 0x00) && ((frame)[3] == 0x01))))

PushAVTransport::PushAVTransport(uint16_t connectionID, TransportTriggerTypeEnum transportTriggerType)
{
    mConnectionID         = connectionID;
    mTransportTriggerType = transportTriggerType;
    mTransportStatus      = TransportStatusEnum::kInactive;
}

void PushAVTransport::initializeRecorder()
{
    if (!isRecorderInitialized)
    {
        recorder = std::make_shared<PushAVClipRecorder>("recorder1", "/workspace/", true, false);
        recorder->Start();
        isRecorderInitialized = true;
    }
}

PushAVTransport::~PushAVTransport()
{
    // TODO cleanup the existing recorded files here.
    mCanSendVideo = false;
    mCanSendAudio = false;
    recorder->Stop();
    // TODO cleanup the existing recorded files here.
    isRecorderInitialized = false;
}

void PushAVTransport::TriggerTransport(TriggerActivationReasonEnum activationReason)
{
    ChipLogProgress(Camera, "PushAVTransport trigger transport, reason: [%u]", (uint16_t) activationReason);
    if (activationReason == TriggerActivationReasonEnum::kUserInitiated)
    {
        ChipLogProgress(Camera, "PushAVTransport trigger transport with activation reason [%u], connectionID: [%u]",
                        (unsigned int) activationReason, mConnectionID);
        StartTransport();
    }
}

void PushAVTransport::StartTransport()
{
    if (mTransportStatus == TransportStatusEnum::kActive)
    {
        initializeRecorder();
        mCanSendVideo = true;
        mCanSendAudio = true;
    }
}

void PushAVTransport::setTransportStatus(TransportStatusEnum status)
{
    if (mTransportStatus == status)
    {
        return;
    }

    mTransportStatus = status;
    if (status == TransportStatusEnum::kInactive)
    {
        mCanSendVideo = false;
        mCanSendAudio = false;
        recorder->Stop();
        // TODO cleanup the existing recorded files here.
        isRecorderInitialized = false;
    }
}

// Implementation of SendVideo method
void PushAVTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    SendAudioVideo(data, size, videoStreamID, 65535);
}

// Implementation of SendAudio method
void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    SendAudioVideo(data, size, 65535, audioStreamID);
}

// Implementation of SendAudioVideo method
void PushAVTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    if (!isRecorderInitialized)
    {
        initializeRecorder();
    }

    AVPacket * packet = createPacket((const uint8_t *) data, size, videoStreamID, audioStreamID);
    if (!packet)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx);
    recorder->PushPacket(packet, videoStreamID != 65535);
}

AVPacket * PushAVTransport::createPacket(const uint8_t * data, int size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    static int found_first = 0;
    AVPacket * packet      = av_packet_alloc();
    packet->data           = (uint8_t *) av_malloc(size);
    if (!packet)
    {
        return nullptr;
    }
    memcpy(packet->data, data, size);
    packet->size = size;
    if (videoStreamID != 65535)
    {
        if (isH264Iframe(data, size))
        {
            found_first   = 1;
            packet->flags = AV_PKT_FLAG_KEY;
        }
        else
        {
            if (found_first == 0)
            {
                return nullptr;
            }
        }
        packet->pts          = v_pts;
        packet->dts          = v_dts;
        packet->stream_index = 0;
        packet->duration     = VID_FRAME_DURATION;
        v_dts += VID_FRAME_DURATION;
        v_pts += VID_FRAME_DURATION; // assuming 20fps for now, adjust as needed
    }
    else
    {
        packet->pts          = a_pts;
        packet->dts          = a_dts;
        packet->stream_index = 1;

        packet->duration = AUD_FRAME_DURATION_48000;
        a_dts += AUD_FRAME_DURATION_48000;
        a_pts += AUD_FRAME_DURATION_48000; // assuming 48kHz for now, adjust as needed
    }

    return packet;
}

// Utility API for Test purpose
void PushAVTransport::readFromFile(char * filename, uint8_t ** videoBuffer, size_t * videoBufferBytes)
{
    const char * in_f_name = filename;
    FILE * infile;
    size_t result;
    /* open an existing file for reading */
    infile = fopen(in_f_name, "r");
    /* quit if the file does not exist */
    if (infile == nullptr)
    {
        return;
    }
    /* Get the number of bytes */
    fseek(infile, 0L, SEEK_END);
    *videoBufferBytes = ftell(infile);

    /* reset the file position indicator to the beginning of the file */
    fseek(infile, 0L, SEEK_SET);
    /* grab sufficient memory for the fileBuffer to hold the text */
    *videoBuffer = (uint8_t *) calloc(*videoBufferBytes, sizeof(uint8_t));
    /* memory error */
    if (*videoBuffer == nullptr)
    {
        fclose(infile);
        return;
    }

    /* copy all the text into the fileBuffer */
    result = fread(*videoBuffer, sizeof(uint8_t), *videoBufferBytes, infile);
    fclose(infile);
    if ((size_t) result != *videoBufferBytes)
    {
        return;
    }
}

bool PushAVTransport::isH264Iframe(const uint8_t * data_ptr, unsigned int data_len)
{
    unsigned int idx = 0;
    int frame_type   = 0;
    int found_sps    = 0;
    int found_pps    = 0;
    int found_idr    = 0;
    int found_pframe = 0;
    bool ret         = false;

    if (data_ptr == nullptr || (data_len < 5))
    {
        return ret;
    }

    do
    {
        if (IS_H264_FRAME_NALU_HEAD(data_ptr + idx))
        {
            if (data_ptr[idx + 2] == 0x01)
                frame_type = data_ptr[idx + 3] & 0x1f;
            else if ((data_ptr[idx + 2] == 0x00) && (data_ptr[idx + 3] == 0x01))
                frame_type = data_ptr[idx + 4] & 0x1f;

            if (frame_type == 7)
            {
                found_sps = 1;
            }
            else if (frame_type == 8)
            {
                found_pps = 1;
            }
            else if (frame_type == 5)
            {
                found_idr = 1;
                break;
            }
            if ((data_ptr[idx + 2] == 0x00) && (data_ptr[idx + 3] == 0x01))
                idx++;

            idx += 4;
        }
        else
        {
            idx++;
        }
    } while (idx < (data_len - 4));

    if (found_sps == 1 && found_pps == 1 && found_idr == 1)
    {
        ret = true;
    }

    return ret;
}

// Implementation of CanSendVideo method
bool PushAVTransport::CanSendVideo()
{
    return mCanSendVideo;
}

// Dummy implementation of CanSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return mCanSendAudio;
}
