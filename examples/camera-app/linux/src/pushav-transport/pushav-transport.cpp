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

#include <pushav-transport/pushav-transport.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

PushAVTransport::PushAVTransport(uint16_t connectionID, const char * url, TransportTriggerTypeEnum transportTriggerType)
{
    mConnectionID         = connectionID;
    mTransportTriggerType = transportTriggerType;
    mTransportStatus      = TransportStatusEnum::kInactive;
    serverUrl             = url;
}

void PushAVTransport::initializeRecorder()
{
    if (!isRecorderInitialized)
    {
        recorder = std::make_unique<PushAVClipRecorder>(clipInfo, audioInfo, videoInfo, serverUrl);
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
    if (!isRecorderInitialized)
    {
        initializeRecorder();
    }
    if (recorder->mRunning==false){
        recorder->Start();
    }
    if(CanSendVideo())
        recorder->PushPacket(data,size, videoStreamID, 1);
}

// Implementation of SendAudio method
void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    if (!isRecorderInitialized)
    {
        initializeRecorder();
    }
    if (recorder->mRunning==false){
        recorder->Start();
    }
    if(CanSendAudio())
        recorder->PushPacket(data,size, audioStreamID, 0);
}

// Implementation of SendAudioVideo method
void PushAVTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    //check if required or remove this method
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
