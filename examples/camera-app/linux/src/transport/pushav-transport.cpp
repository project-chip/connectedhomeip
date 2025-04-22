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
    #include<libavcodec/avcodec.h>
    #include<libavutil/avutil.h>
}

#define AUD_FRAME_DURATION_48000 960
#define VID_FRAME_DURATION 5000 // Microseconds, for 20fps

#define IS_H264_FRAME_NALU_HEAD(frame) (((frame)[0] == 0x00) && \
                                        ((frame)[1] == 0x00) && \
                                        (((frame)[2] == 0x01) || (((frame)[2] == 0x00) && ((frame)[3] == 0x01))))


PushAVTransport::PushAVTransport(uint16_t sessionID, uint64_t nodeID) {
    mCanSendVideo = true;
}

void PushAVTransport::initializeRecorder(){
    if(!isRecorderInitialized) {
       recorder = std::make_shared<PushAVClipRecorder>("recorder1", "/workspace/", true, false);
       recorder->Start();
       isRecorderInitialized = true;
    }
}

// Destructor definition
PushAVTransport::~PushAVTransport()
{
    // Destructor implementation
}
// Dummy implementation of SendVideo method
void PushAVTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    // Placeholder for actual PushAV implementation to send video data
    SendAudioVideo(data, size, videoStreamID, 65535);
}

// Dummy implementation of SendAudio method
void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
     mCanSendAudio = true;
     // Placeholder for actual PushAV implementation to send audio data
     SendAudioVideo(data, size, 65535, audioStreamID);
}

// Dummy implementation of SendAudioVideo method
void PushAVTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual PushAV implementation to send synchronized audio/video data
    if(!isRecorderInitialized) {
       initializeRecorder();
    }
/*  // Enable below code to send data from .h264 files
    char filename[2048];
    uint8_t* filedata = nullptr;
    size_t dataLen = 0;
    
    const char path[2048] = "/workspace/cfmv/";
    if (vid == 203) vid = 1;
    sprintf(filename, "%s%d.h264",path, vid++);
    readFromFile(filename, &filedata, &dataLen);

    AVPacket* packet = createPacket(filedata, dataLen, videoStreamID, audioStreamID);*/
    AVPacket* packet = createPacket((const uint8_t*)data, size, videoStreamID, audioStreamID);
    if (!packet) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx);
    recorder->PushPacket(packet, videoStreamID!= 65535);     
}

AVPacket* PushAVTransport::createPacket(const uint8_t* data, int size,  uint16_t videoStreamID, uint16_t audioStreamID) {
    AVPacket *packet = av_packet_alloc();
    packet->data = (uint8_t*)av_malloc(size);
    if (!packet) {
        return nullptr;
    }
    memcpy(packet->data, data, size);
    packet->size = size;
    if(videoStreamID != 65535) {
        packet->pts = v_pts;
        packet->dts = v_dts;
        packet->stream_index = 0;
        if(isH264Iframe(data, size)){
            packet->flags = AV_PKT_FLAG_KEY;
        }
        v_dts += VID_FRAME_DURATION;
        v_pts += VID_FRAME_DURATION; //assuming 20fps for now, adjust as needed
    }
    else{
        packet->pts = a_pts;
        packet->dts = a_dts;
        packet->stream_index = 1;

        a_dts += AUD_FRAME_DURATION_48000;
        a_pts += AUD_FRAME_DURATION_48000; //assuming 48kHz for now, adjust as needed
    }

    return packet;
}

void PushAVTransport::readFromFile(char* filename, uint8_t** videoBuffer, size_t *videoBufferBytes)
{
    const char *in_f_name = filename;
    FILE *infile;
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
    *videoBuffer = (uint8_t *)calloc(*videoBufferBytes, sizeof(uint8_t));
    /* memory error */
    if (*videoBuffer == nullptr)
    {
        fclose(infile);
        return;
    }

    /* copy all the text into the fileBuffer */
    result = fread(*videoBuffer, sizeof(uint8_t), *videoBufferBytes, infile);
    fclose(infile);
    if ((size_t)result != *videoBufferBytes)
    {
        return;
    }
}

bool PushAVTransport::isH264Iframe(const uint8_t *data_ptr, unsigned int data_len)
{
    unsigned int idx = 0;
    int frame_type = 0;
    int found_sps = 0;
    int found_pps = 0;
    int found_idr = 0;
    int found_pframe = 0;
    bool ret = false;

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

// Dummy implementation of CanSendVideo method
bool PushAVTransport::CanSendVideo()
{
    return mCanSendVideo;
}

// Dummy implementation of CanSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return mCanSendAudio;
}
