/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#ifndef ContentLauncherTypes_h
#define ContentLauncherTypes_h

typedef NS_ENUM(uint8_t, ContentLauncher_ParameterEnum) {
    Actor = 0x00,
    Channel = 0x01,
    Character = 0x02,
    Director = 0x03,
    Event = 0x04,
    Franchise = 0x05,
    Genre = 0x06,
    League = 0x07,
    Popularity = 0x08,
    Provider = 0x09,
    Sport = 0x10,
    SportsTeam = 0x11,
    Type = 0x12,
    Video = 0x13,
};

@interface ContentLauncher_AdditionalInfo : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSString * _Nonnull value;

- (ContentLauncher_AdditionalInfo * _Nonnull)initWithName:(NSString * _Nonnull)name value:(NSString * _Nonnull)value;
- (id _Nonnull)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface ContentLauncher_Parameter : NSObject <NSCopying>

@property (nonatomic) ContentLauncher_ParameterEnum type;
@property (nonatomic, copy) NSString * _Nonnull value;
@property (nonatomic, copy) NSArray * _Nullable externalIDList;

- (ContentLauncher_Parameter * _Nonnull)initWithType:(ContentLauncher_ParameterEnum)type
                                               value:(NSString * _Nonnull)value
                                      externalIDList:(NSArray * _Nullable)externalIDList;
- (id _Nonnull)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface ContentLauncher_ContentSearch : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull parameterList;

- (ContentLauncher_ContentSearch * _Nonnull)initWithParameterList:(NSArray * _Nonnull)parameterList;
- (id _Nonnull)copyWithZone:(NSZone * _Nullable)zone;
@end
#endif /* ContentLauncherTypes_h */
