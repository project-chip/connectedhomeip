/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <jni.h>

#include "Constants.h"
#include "MatterCallbackHandler-JNI.h"

class TvCastingAppJNI
{
public:
    MatterCallbackHandlerJNI & getCommissioningCompleteHandler() { return mCommissioningCompleteHandler; }

    SessionEstablishmentStartedHandlerJNI & getSessionEstablishmentStartedHandler() { return mSessionEstablishmentStartedHandler; }

    SessionEstablishedHandlerJNI & getSessionEstablishedHandler() { return mSessionEstablishedHandler; }

    FailureHandlerJNI & getSessionEstablishmentErrorHandler() { return mSessionEstablishmentErrorHandler; }

    FailureHandlerJNI & getSessionEstablishmentStoppedHandler() { return mSessionEstablishmentStoppedHandler; }

    OnConnectionSuccessHandlerJNI & getOnConnectionSuccessHandler(bool preCommissioned)
    {
        return preCommissioned ? mPreCommissionedOnConnectionSuccessHandler : mCommissioningOnConnectionSuccessHandler;
    }

    FailureHandlerJNI & getOnConnectionFailureHandler(bool preCommissioned)
    {
        return preCommissioned ? mPreCommissionedOnConnectionFailureHandler : mCommissioningOnConnectionFailureHandler;
    }
    OnNewOrUpdatedEndpointHandlerJNI & getOnNewOrUpdatedEndpointHandler(bool preCommissioned)
    {
        return preCommissioned ? mPreCommissionedOnNewOrUpdatedEndpointHandler : mCommissioningOnNewOrUpdatedEndpointHandler;
    }

    MatterCallbackHandlerJNI & getMediaCommandResponseHandler(enum MediaCommandName name)
    {
        return mMediaCommandResponseHandler[name];
    }

    FailureHandlerJNI & getReadFailureHandler(enum MediaAttributeName name) { return mReadFailureHandler[name]; }

    FailureHandlerJNI & getSubscriptionReadFailureHandler(enum MediaAttributeName name)
    {
        return mSubscriptionReadFailureHandler[name];
    }

    SubscriptionEstablishedHandlerJNI & getSubscriptionEstablishedHandler(enum MediaAttributeName name)
    {
        return mSubscriptionEstablishedHandler[name];
    }

    CurrentStateSuccessHandlerJNI & getCurrentStateSuccessHandler() { return mCurrentStateSuccessHandlerJNI; }
    DurationSuccessHandlerJNI & getDurationSuccessHandler() { return mDurationSuccessHandlerJNI; }
    SampledPositionSuccessHandlerJNI & getSampledPositionSuccessHandler() { return mSampledPositionSuccessHandlerJNI; }
    PlaybackSpeedSuccessHandlerJNI & getPlaybackSpeedSuccessHandler() { return mPlaybackSpeedSuccessHandlerJNI; }
    SeekRangeEndSuccessHandlerJNI & getSeekRangeEndSuccessHandler() { return mSeekRangeEndSuccessHandlerJNI; }
    SeekRangeStartSuccessHandlerJNI & getSeekRangeStartSuccessHandler() { return mSeekRangeStartSuccessHandlerJNI; }

    CurrentTargetSuccessHandlerJNI & getCurrentTargetSuccessHandler() { return mCurrentTargetSuccessHandlerJNI; }
    TargetListSuccessHandlerJNI & getTargetListSuccessHandler() { return mTargetListSuccessHandlerJNI; }

    CurrentLevelSuccessHandlerJNI & getCurrentLevelSuccessHandler() { return mCurrentLevelSuccessHandlerJNI; }
    MinLevelSuccessHandlerJNI & getMinLevelSuccessHandler() { return mMinLevelSuccessHandlerJNI; }
    MaxLevelSuccessHandlerJNI & getMaxLevelSuccessHandler() { return mMaxLevelSuccessHandlerJNI; }

    SupportedStreamingProtocolsSuccessHandlerJNI & getSupportedStreamingProtocolsSuccessHandler()
    {
        return mSupportedStreamingProtocolsSuccessHandlerJNI;
    }

    VendorNameSuccessHandlerJNI & getVendorNameSuccessHandler() { return mVendorNameSuccessHandlerJNI; }
    VendorIDSuccessHandlerJNI & getVendorIDSuccessHandler() { return mVendorIDSuccessHandlerJNI; }
    ApplicationNameSuccessHandlerJNI & getApplicationNameSuccessHandler() { return mApplicationNameSuccessHandlerJNI; }
    ProductIDSuccessHandlerJNI & getProductIDSuccessHandler() { return mProductIDSuccessHandlerJNI; }
    ApplicationVersionSuccessHandlerJNI & getApplicationVersionSuccessHandler() { return mApplicationVersionSuccessHandlerJNI; }

    VendorNameSuccessHandlerJNI & getReadVendorNameSuccessHandler() { return mReadVendorNameSuccessHandlerJNI; }
    VendorIDSuccessHandlerJNI & getReadVendorIDSuccessHandler() { return mReadVendorIDSuccessHandlerJNI; }
    ApplicationNameSuccessHandlerJNI & getReadApplicationNameSuccessHandler() { return mReadApplicationNameSuccessHandlerJNI; }
    ProductIDSuccessHandlerJNI & getReadProductIDSuccessHandler() { return mReadProductIDSuccessHandlerJNI; }
    ApplicationVersionSuccessHandlerJNI & getReadApplicationVersionSuccessHandler()
    {
        return mReadApplicationVersionSuccessHandlerJNI;
    }

    jboolean runCastingServerCommand(JNIEnv * env, jobject contentApp, jobject jResponseHandler, const char * commandName,
                                     MediaCommandName command, const std::function<CHIP_ERROR(TargetEndpointInfo)> & commandRunner);

private:
    friend TvCastingAppJNI & TvCastingAppJNIMgr();

    static TvCastingAppJNI sInstance;

    MatterCallbackHandlerJNI mCommissioningCompleteHandler;
    SessionEstablishmentStartedHandlerJNI mSessionEstablishmentStartedHandler;
    SessionEstablishedHandlerJNI mSessionEstablishedHandler;
    FailureHandlerJNI mSessionEstablishmentErrorHandler;
    FailureHandlerJNI mSessionEstablishmentStoppedHandler;
    OnConnectionSuccessHandlerJNI mCommissioningOnConnectionSuccessHandler;
    FailureHandlerJNI mCommissioningOnConnectionFailureHandler;
    OnNewOrUpdatedEndpointHandlerJNI mCommissioningOnNewOrUpdatedEndpointHandler;

    OnConnectionSuccessHandlerJNI mPreCommissionedOnConnectionSuccessHandler;
    FailureHandlerJNI mPreCommissionedOnConnectionFailureHandler;
    OnNewOrUpdatedEndpointHandlerJNI mPreCommissionedOnNewOrUpdatedEndpointHandler;

    MatterCallbackHandlerJNI mMediaCommandResponseHandler[MEDIA_COMMAND_COUNT];
    FailureHandlerJNI mSubscriptionReadFailureHandler[MEDIA_ATTRIBUTE_COUNT];
    SubscriptionEstablishedHandlerJNI mSubscriptionEstablishedHandler[MEDIA_ATTRIBUTE_COUNT];
    FailureHandlerJNI mReadFailureHandler[MEDIA_ATTRIBUTE_COUNT];

    CurrentStateSuccessHandlerJNI mCurrentStateSuccessHandlerJNI;
    DurationSuccessHandlerJNI mDurationSuccessHandlerJNI;
    SampledPositionSuccessHandlerJNI mSampledPositionSuccessHandlerJNI;
    PlaybackSpeedSuccessHandlerJNI mPlaybackSpeedSuccessHandlerJNI;
    SeekRangeEndSuccessHandlerJNI mSeekRangeEndSuccessHandlerJNI;
    SeekRangeStartSuccessHandlerJNI mSeekRangeStartSuccessHandlerJNI;

    CurrentTargetSuccessHandlerJNI mCurrentTargetSuccessHandlerJNI;
    TargetListSuccessHandlerJNI mTargetListSuccessHandlerJNI;

    CurrentLevelSuccessHandlerJNI mCurrentLevelSuccessHandlerJNI;
    MinLevelSuccessHandlerJNI mMinLevelSuccessHandlerJNI;
    MaxLevelSuccessHandlerJNI mMaxLevelSuccessHandlerJNI;

    SupportedStreamingProtocolsSuccessHandlerJNI mSupportedStreamingProtocolsSuccessHandlerJNI;

    VendorNameSuccessHandlerJNI mVendorNameSuccessHandlerJNI;
    VendorIDSuccessHandlerJNI mVendorIDSuccessHandlerJNI;
    ApplicationNameSuccessHandlerJNI mApplicationNameSuccessHandlerJNI;
    ProductIDSuccessHandlerJNI mProductIDSuccessHandlerJNI;
    ApplicationVersionSuccessHandlerJNI mApplicationVersionSuccessHandlerJNI;

    VendorNameSuccessHandlerJNI mReadVendorNameSuccessHandlerJNI;
    VendorIDSuccessHandlerJNI mReadVendorIDSuccessHandlerJNI;
    ApplicationNameSuccessHandlerJNI mReadApplicationNameSuccessHandlerJNI;
    ProductIDSuccessHandlerJNI mReadProductIDSuccessHandlerJNI;
    ApplicationVersionSuccessHandlerJNI mReadApplicationVersionSuccessHandlerJNI;
};

inline class TvCastingAppJNI & TvCastingAppJNIMgr()
{
    return TvCastingAppJNI::sInstance;
}
