/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
