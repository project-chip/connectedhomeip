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

#import "MCErrorUtils.h"

#include <lib/core/CHIPError.h>

@implementation MCErrorUtils

+ (MatterError * _Nonnull)MatterErrorFromChipError:(CHIP_ERROR)chipError
{
    return [[MatterError alloc] initWithCode:chipError.AsInteger() message:[NSString stringWithUTF8String:chipError.AsString()]];
}

+ (NSError * _Nonnull)NSErrorFromChipError:(CHIP_ERROR)chipError
{
    return chipError == CHIP_NO_ERROR ? nil : [NSError errorWithDomain:@"com.matter.casting" code:chipError.AsInteger() userInfo:@{ NSUnderlyingErrorKey : [NSString stringWithUTF8String:chipError.AsString()] }];
}

+ (NSError * _Nonnull)NSErrorFromMatterError:(MatterError * _Nonnull)matterError
{
    return matterError == MATTER_NO_ERROR ? nil : [NSError errorWithDomain:@"com.matter.casting" code:matterError.code userInfo:@{ NSUnderlyingErrorKey : matterError.message }];
}

+ (MatterError * _Nonnull)MatterErrorFromNsError:(NSError * _Nonnull)nsError
{
    return nsError == nil ? MATTER_NO_ERROR : [[MatterError alloc] initWithCode:static_cast<uint32_t>(nsError.code) message:nsError.userInfo[NSUnderlyingErrorKey]];
}
@end
