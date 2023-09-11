/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#import <Foundation/Foundation.h>

#ifndef MediaPlaybackTypes_h
#define MediaPlaybackTypes_h

typedef NS_ENUM(uint8_t, MediaPlayback_PlaybackState) {
    Playing = 0x00,
    Paused = 0x01,
    NotPlaying = 0x02,
    Buffering = 0x03,
};

@interface MediaPlayback_PlaybackPosition : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull updatedAt;
@property (nonatomic, copy) NSNumber * _Nullable position;

- (MediaPlayback_PlaybackPosition * _Nonnull)initWithUpdatedAt:(NSNumber * _Nonnull)updatedAt
                                                      position:(NSNumber * _Nullable)position;
- (id _Nonnull)copyWithZone:(NSZone * _Nullable)zone;
@end

#endif /* MediaPlaybackTypes_h */
