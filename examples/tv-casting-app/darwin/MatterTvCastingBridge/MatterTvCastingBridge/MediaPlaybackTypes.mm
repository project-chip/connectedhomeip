/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MediaPlaybackTypes.h"

@implementation MediaPlayback_PlaybackPosition

- (MediaPlayback_PlaybackPosition * _Nonnull)initWithUpdatedAt:(NSNumber * _Nonnull)updatedAt
                                                      position:(NSNumber * _Nullable)position
{
    self = [super init];
    if (self) {
        _updatedAt = updatedAt;
        _position = position;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MediaPlayback_PlaybackPosition: updatedAt=%@ position=%@", _updatedAt, _position];
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    return [[MediaPlayback_PlaybackPosition alloc] initWithUpdatedAt:_updatedAt position:_position];
}
@end
