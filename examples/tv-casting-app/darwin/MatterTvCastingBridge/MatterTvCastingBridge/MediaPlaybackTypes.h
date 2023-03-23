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
