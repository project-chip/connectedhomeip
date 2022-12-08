/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#ifndef VideoPlayer_h
#define VideoPlayer_h

@interface VideoPlayer : NSObject

@property uint64_t nodeId;

@property uint8_t fabricIndex;

/**
 * @brief true if this VideoPlayer is connected, false otherwise
 */
@property bool isConnected;

/**
 * @brief contentApps will be nil the VideoPlayer is not connected
 */
@property NSMutableArray * contentApps;

@property NSString * deviceName;

@property uint16_t vendorId;

@property uint16_t productId;

@property uint16_t deviceType;

/**
 @brief true, if all the required fields are initialized, false otherwise
 */
@property BOOL isInitialized;

- (instancetype)initWithNodeId:(uint64_t)nodeId
                   fabricIndex:(uint8_t)fabricIndex
                   isConnected:(bool)isConnected
                   contentApps:(NSMutableArray *)contentApps
                    deviceName:(NSString *)deviceName
                      vendorId:(uint16_t)vendorId
                     productId:(uint16_t)productId
                    deviceType:(uint16_t)deviceType;

@end

#endif /* VideoPlayer_h */
