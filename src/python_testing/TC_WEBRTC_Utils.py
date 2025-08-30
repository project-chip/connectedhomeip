#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import logging

from matter.clusters import CameraAvStreamManagement


class WebRTCTestHelper:
    async def read_avstr_attribute_expect_success(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=CameraAvStreamManagement, attribute=attribute
        )

    async def allocate_video_stream(self, endpoint):
        """Try to allocate a video stream from the camera device. Returns the stream ID if successful, otherwise None."""
        attrs = CameraAvStreamManagement.Attributes
        try:
            # Check for watermark and OSD features
            feature_map = await self.read_avstr_attribute_expect_success(endpoint, attrs.FeatureMap)
            watermark = True if (feature_map & CameraAvStreamManagement.Bitmaps.Feature.kWatermark) != 0 else None
            osd = True if (feature_map & CameraAvStreamManagement.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

            # Get the parms from the device (those which are available)
            aStreamUsagePriorities = await self.read_avstr_attribute_expect_success(endpoint, attrs.StreamUsagePriorities)
            aRateDistortionTradeOffPoints = await self.read_avstr_attribute_expect_success(
                endpoint, attrs.RateDistortionTradeOffPoints
            )
            aMinViewportRes = await self.read_avstr_attribute_expect_success(endpoint, attrs.MinViewportResolution)
            aVideoSensorParams = await self.read_avstr_attribute_expect_success(endpoint, attrs.VideoSensorParams)

            response = await self.send_single_cmd(
                cmd=CameraAvStreamManagement.Commands.VideoStreamAllocate(
                    streamUsage=aStreamUsagePriorities[0],
                    videoCodec=aRateDistortionTradeOffPoints[0].codec,
                    minFrameRate=30,
                    maxFrameRate=aVideoSensorParams.maxFPS,
                    minResolution=aMinViewportRes,
                    maxResolution=CameraAvStreamManagement.Structs.VideoResolutionStruct(
                        width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                    ),
                    minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    keyFrameInterval=4000,
                    watermarkEnabled=watermark,
                    OSDEnabled=osd,
                ),
                endpoint=endpoint,
            )
            return response.videoStreamID

        except Exception as e:
            logging.error(f"Failed to allocate video stream. {e}")
            return None
