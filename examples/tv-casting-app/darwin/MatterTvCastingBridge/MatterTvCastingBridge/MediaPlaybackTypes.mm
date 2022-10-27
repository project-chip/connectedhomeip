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
